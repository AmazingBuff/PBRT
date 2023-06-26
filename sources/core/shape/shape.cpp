//
// Created by AmazingBuff on 2023/6/21.
//

#include "shape.h"
#include "core/transform/transform.h"
#include "core/interaction/interaction.h"
#include "core/memory/memory.h"

#include <set>
#include <map>

namespace pbrt
{
	Shape::Shape(const Transform* ObjectToWorld, const Transform* WorldToObject, bool reverseOrientation)
	: ObjectToWorld(ObjectToWorld), WorldToObject(WorldToObject), reverseOrientation(reverseOrientation),
	  transformSwapHandedness(ObjectToWorld->IsSwapHandedness()) {}

	Bounds3f Shape::WorldBound() const
	{
		return ObjectToWorld->operator()(ObjectBound());
	}

	bool Shape::IntersectP(const Ray& ray, bool testAlphaTexture) const
	{
		Float tHit = ray.tMax;
		SurfaceInteraction surfaceInteraction;
		return Intersect(ray, &tHit, &surfaceInteraction, testAlphaTexture);
	}


	uint32_t SDFace::VertexIndex(const SDVertex* vertex) const
	{
		for(uint32_t i = 0; i < 3; i++)
		{
			if(vertices[i] == vertex)
				return i;
		}
		Fatal("Basic logic error");
		return std::numeric_limits<uint32_t>::max();
	}

	SDVertex* SDFace::OtherVertex(const SDVertex* v0, const SDVertex* v1) const
	{
		for(uint32_t i = 0; i < 3; i++)
		{
			if(vertices[i] != v0 && vertices[i] != v1)
				return vertices[i];
		}
		Fatal("Basic logic error");
		return nullptr;
	}


	uint32_t SDVertex::Valence() const
	{
		SDFace* f = startFace;
		if(!isBoundary)
		{
			//compute valence of interior vertex
			uint32_t nf = 1;
			while((f = f->NextFace(this)) != startFace)
				nf++;
			return nf;
		}
		else
		{
			//compute valence of boundary vertex
			uint32_t nf = 1;
			while((f = f->NextFace(this)) != nullptr)
				nf++;
			f = startFace;
			while((f = f->PrevFace(this)) != nullptr)
				nf++;
			return nf + 1;
		}
	}

	void SDVertex::OneRing(Point3f* point)
	{
		if(!isBoundary)
		{
			SDFace* face = startFace;
			do{
				*point++ = face->NextVertex(this)->position;
				face = face->NextFace(this);
			} while(face != startFace);
		}
		else
		{
			SDFace* face = startFace;
			SDFace* f;
			while ((f = face->NextFace(this)) != nullptr)
				face = f;
			*point++ = face->NextVertex(this)->position;
			do{
				*point++ = face->PrevVertex(this)->position;
				face = face->PrevFace(this);
			} while(face != nullptr);
		}
	}


	static Point3f WeightOneRing(SDVertex* vertex, Float beta)
	{
		//put vertex one-ring in pRing
		uint32_t valence = vertex->Valence();
		Point3f* pRing = ALLOCA(Point3f, valence);
		vertex->OneRing(pRing);

		Point3f p = (1.f - valence * beta) * vertex->position;
		for(uint32_t i = 0; i < valence; i++)
			p += beta * pRing[i];
		return p;
	}

	static Point3f WeightBoundary(SDVertex* vertex, Float beta)
	{
		//put vertex one-ring in pRing
		uint32_t valence = vertex->Valence();
		Point3f* pRing = ALLOCA(Point3f, valence);
		vertex->OneRing(pRing);

		Point3f p = (1.f - 2.f * beta) * vertex->position;
		p += beta * pRing[0];
		p += beta * pRing[valence - 1];
		return p;
	}

	std::vector<std::shared_ptr<Shape>>
	LoopSubdivide(const Transform* ObjectToWorld, const Transform* WorldToObject,
				  bool reverseOrientation, uint32_t nLevels, uint32_t nIndices,
				  const uint32_t* vertexIndices, uint32_t nVertices, const Point3f* point)
	{
		std::vector<SDVertex*> vertices;
		std::vector<SDFace*> faces;
		//allocate vertices and faces
		std::unique_ptr<SDVertex[]> vertex(new SDVertex[nVertices]);
		for(uint32_t i = 0; i < nVertices; i++)
		{
			vertex[i] = SDVertex(point[i]);
			vertices.push_back(&vertex[i]);
		}
		uint32_t nFaces = nIndices / 3;
		std::unique_ptr<SDFace[]> face(new SDFace[nFaces]);
		for(uint32_t i = 0; i < nFaces; i++)
			faces.push_back(&face[i]);
		//set face to vertex pointers
		const uint32_t* vertexPos = vertexIndices;
		for(uint32_t i = 0; i < nFaces; i++, vertexPos += 3)
		{
			SDFace* f = faces[i];
			for(uint32_t j = 0; j < 3; j++)
			{
				SDVertex* v = vertices[vertexPos[j]];
				f->vertices[j] = v;
				v->startFace = f;
			}
		}
		//set neighbor pointers in faces
		std::set<SDEdge> edges;
		for(uint32_t i = 0; i < nFaces; i++)
		{
			SDFace* f = faces[i];
			for(uint32_t edge = 0; edge < 3; edge++)
			{
				//update neighbor pointer for edge
				uint32_t v0 = edge, v1 = Next(edge);
				SDEdge e(f->vertices[v0], f->vertices[v1]);
				if(edges.find(e) == edges.end())
				{
					//handle new edge
					e.faces[0] = f;
					e.edgeNum = edge;
					edges.insert(e);
				}
				else
				{
					//handle previously seen edge
					e = *edges.find(e);
					e.faces[0]->faces[e.edgeNum] = f;
					f->faces[edge] = e.faces[0];
					edges.erase(e);
				}
			}
		}
		//finish vertex initialization
		for(uint32_t i = 0; i < nVertices; i++)
		{
			SDVertex* v = vertices[i];
			SDFace* f = v->startFace;
			do{
				f = f->NextFace(v);
			} while(f && f != v->startFace);
			v->isBoundary = (f == nullptr);
			if((!v->isBoundary && v->Valence() == 6) || (v->isBoundary && v->Valence() == 4))
				v->isRegular = true;
			else
				v->isRegular = false;
		}
		//refine subdivision mesh into triangles
		std::vector<SDFace*> f = faces;
		std::vector<SDVertex*> v = vertices;
		MemoryArena arena;
		for(uint32_t i = 0; i < nLevels; i++)
		{
			//update f and v for next level of subdivision
			std::vector<SDFace*> newFaces;
			std::vector<SDVertex*> newVertices;
			//allocate next level of children in mesh tree
			for(SDVertex* vertexIt : v)
			{
				vertexIt->child = arena.Alloc<SDVertex>();
				vertexIt->child->isRegular = vertexIt->isRegular;
				vertexIt->child->isBoundary = vertexIt->isBoundary;
				newVertices.push_back(vertexIt->child);
			}
			for(SDFace* faceIt : f)
			{
				for(uint32_t j = 0; j < 4; j++)
				{
					faceIt->children[j] = arena.Alloc<SDFace>();
					newFaces.push_back(faceIt->children[j]);
				}
			}
			//update vertex positions and create new edge vertices
			//update vertex positions for even vertices
			for(SDVertex* vertexIt : v)
			{
				if(!vertexIt->isBoundary)
				{
					//apply one-ring rule
					if(vertexIt->isRegular)
						vertexIt->child->position = WeightOneRing(vertexIt, 1.f / 16.f);
					else
						vertexIt->child->position = WeightOneRing(vertexIt, Beta(vertexIt->Valence()));
				}
				else
				{
					//apply boundary rule
					vertexIt->child->position = WeightBoundary(vertexIt, 1.f / 8.f);
				}
			}
			//compute new odd edge vertices
			std::map<SDEdge, SDVertex*> edgeVertices;
			for(SDFace* faceIt : f)
			{
				for(uint32_t k = 0; k < 3; k++)
				{
					//compute odd vertex on kth edge
					SDEdge edge(faceIt->vertices[k], faceIt->vertices[Next(k)]);
					if(edgeVertices.find(edge) == edgeVertices.end())
					{
						//create and initialize new odd vertex
						SDVertex* vertexValue = arena.Alloc<SDVertex>();
						newVertices.push_back(vertexValue);
						vertexValue->isRegular = true;
						vertexValue->isBoundary = (faceIt->faces[k] == nullptr);
						vertexValue->startFace = faceIt->children[3];
						//apply edge rules to compute new vertex position
						if(vertexValue->isBoundary)
						{
							vertexValue->position = 0.5f * edge.vertices[0]->position;
							vertexValue->position += 0.5f * edge.vertices[1]->position;
						}
						else
						{
							vertexValue->position = 3.f / 8.f * edge.vertices[0]->position;
							vertexValue->position += 3.f / 8.f * edge.vertices[1]->position;
							vertexValue->position +=
								1.f / 8.f * faceIt->OtherVertex(edge.vertices[0], edge.vertices[1])->position;
							vertexValue->position +=
								1.f / 8.f * faceIt->faces[k]->OtherVertex(edge.vertices[0], edge.vertices[1])->position;
						}
						edgeVertices[edge] = vertexValue;
					}
				}
			}
			//update new mesh topology
			//update even vertex face pointers
			for(SDVertex* vertexIt : v)
			{
				uint32_t vIndex = vertexIt->startFace->VertexIndex(vertexIt);
				vertexIt->child->startFace = vertexIt->startFace->children[vIndex];
			}
			//update face neighbor pointers
			for(SDFace* faceIt : f)
			{
				for(uint32_t j = 0; j < 3; j++)
				{
					//update children f pointers for siblings
					faceIt->children[3]->faces[j] = faceIt->children[Next(j)];
					faceIt->children[j]->faces[Next(j)] = faceIt->children[3];
					//update children f pointers for neighbor children
					SDFace* faceOther = faceIt->faces[j];
					faceIt->children[j]->faces[j] =
						faceOther ? faceOther->children[faceOther->VertexIndex(faceIt->vertices[j])] : nullptr;
					faceOther = faceIt->faces[Prev(j)];
					faceIt->children[j]->faces[Prev(j)] =
						faceOther ? faceOther->children[faceOther->VertexIndex(faceIt->vertices[j])] : nullptr;
				}
			}
			//update face vertex pointers
			for(SDFace* faceIt : f)
			{
				for(uint32_t j = 0; j < 3; j++)
				{
					//update child vertex pointer to new even vertex
					faceIt->children[j]->vertices[j] = faceIt->vertices[j]->child;
					//update child vertex pointer to new odd vertex
					SDVertex* vertexIt = edgeVertices[SDEdge(faceIt->vertices[j], faceIt->vertices[Next(j)])];
					faceIt->children[j]->vertices[Next(j)] = vertexIt;
					faceIt->children[Next(j)]->vertices[j] = vertexIt;
					faceIt->children[3]->vertices[j] = vertexIt;
				}
			}
			//prepare for next level of subdivision
			f = newFaces;
			v = newVertices;
		}
		//push vertices to limit surface
		std::unique_ptr<Point3f[]> pLimit(new Point3f[v.size()]);
		for(size_t i = 0; i < v.size(); i++)
		{
			if(v[i]->isBoundary)
				pLimit[i] = WeightBoundary(v[i], 1.f / 5.f);
			else
				pLimit[i] = WeightOneRing(v[i], LoopGamma(v[i]->Valence()));
		}
		for(size_t i = 0; i < v.size(); i++)
			v[i]->position = pLimit[i];
		//compute vertex tangents on limit surface
		std::vector<Normal3f> normals;
		normals.reserve(v.size());
		std::vector<Point3f> pRing(16, Point3f());
		for(SDVertex* vertexIt : v)
		{
			Vector3f tangent(0.f, 0.f, 0.f), bitangent(0.f, 0.f, 0.f);
			uint32_t valence = vertexIt->Valence();
			if(valence > static_cast<uint32_t>(pRing.size()))
				pRing.resize(valence);
			vertexIt->OneRing(pRing.data());
			if(!vertexIt->isBoundary)
			{
				for(uint32_t j = 0; j < valence; j++)
				{
					tangent += std::cos(2.f * Pi * j / valence) * Vector3f(pRing[j]);
					bitangent += std::sin(2.f * Pi * j / valence) * Vector3f(pRing[j]);
				}
			}
			else
			{
				tangent = pRing[valence - 1] - pRing[0];
				if(valence == 2)
					bitangent = Vector3f(pRing[0] + pRing[1] - 2.f * vertexIt->position);
				else if(valence == 3)
					bitangent = pRing[1] - vertexIt->position;
				else if(valence == 4)
					bitangent = Vector3f(-2.f * vertexIt->position + 2.f * pRing[1] + 2.f * pRing[2] - (pRing[0] + pRing[3]));
				else
				{
					Float theta = Pi / static_cast<Float>(valence - 1);
					bitangent = Vector3f(std::sin(theta) * (pRing[0] + pRing[valence - 1]));
					for(uint32_t i = 1; i < valence - 1; i++)
					{
						Float w = (2.f * std::cos(theta) - 2.f) * std::sin(static_cast<Float>(i) * theta);
						bitangent += Vector3f(w * pRing[i]);
					}
					bitangent = -bitangent;
				}
			}
			normals.push_back(Normal3f(Cross(tangent, bitangent)));
		}
		//create triangle mesh from subdivision mesh
	}

} // pbrt