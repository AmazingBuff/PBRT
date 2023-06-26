//
// Created by AmazingBuff on 2023/6/21.
//

#pragma once
#include "core/pbrt.h"
#include "core/geometry/geometry.h"


namespace pbrt
{

	class Shape
	{
	public:
		const Transform* ObjectToWorld, *WorldToObject;
		const bool reverseOrientation;
		const bool transformSwapHandedness;
	public:
		Shape(const Transform* ObjectToWorld, const Transform* WorldToObject, bool reverseOrientation);

		virtual Bounds3f ObjectBound() const = 0;
		virtual Bounds3f WorldBound() const;
		virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
							   bool testAlphaTexture) const = 0;
		virtual bool IntersectP(const Ray& ray, bool testAlphaTexture = true) const;
		virtual Float Area() const = 0;
	};



	template<typename T>
	inline T Next(T i)
	{
		return (i + 1) % 3;
	}

	template<typename T>
	inline T Prev(T i)
	{
		return (i + 2) % 3;
	}

	inline Float Beta(uint32_t valence)
	{
		if(valence == 3)
			return 3.f / 16.f;
		else
			return 3.f / (8.f * valence);
	}

	inline Float LoopGamma(uint32_t valence)
	{
		return 1.f / (valence + 3.f / (8.f * Beta(valence)));
	}

	class SDVertex;

	class SDFace
	{
	public:
		std::array<SDVertex*, 3> vertices;
		std::array<SDFace*, 3> faces;
		std::array<SDFace*, 4> children;

	public:
		uint32_t VertexIndex(const SDVertex* vertex) const;
		SDVertex* OtherVertex(const SDVertex* v0, const SDVertex* v1) const;

		SDFace* NextFace(const SDVertex* vertex) const
		{
			return faces[VertexIndex(vertex)];
		}

		SDFace* PrevFace(const SDVertex* vertex) const
		{
			return faces[Prev(VertexIndex(vertex))];
		}

		SDVertex* NextVertex(const SDVertex* vertex) const
		{
			return vertices[Next(VertexIndex(vertex))];
		}

		SDVertex* PrevVertex(const SDVertex* vertex) const
		{
			return vertices[Prev(VertexIndex(vertex))];
		}
	};


	class SDVertex
	{
	public:
		Point3f position;
		SDFace* startFace = nullptr;
		SDVertex* child = nullptr;
		bool isRegular = false, isBoundary = false;
	public:
		SDVertex(const Point3f& pos = Point3f(0.f, 0.f, 0.f)) : position(pos) {}

		uint32_t Valence() const;
		void OneRing(Point3f* point);
	};


	class SDEdge
	{
	public:
		std::array<SDVertex*, 2> vertices;
		std::array<SDFace*, 2> faces;
		uint32_t edgeNum;
	public:
		SDEdge(SDVertex* v0 = nullptr, SDVertex* v1 = nullptr)
		{
			vertices[0] = std::min(v0, v1);
			vertices[1] = std::max(v0, v1);
			faces[0] = faces[1] = nullptr;
			edgeNum = std::numeric_limits<uint32_t>::max();
		}
	public:
		bool operator<(const SDEdge& other) const
		{
			if(vertices[0] == other.vertices[0])
				return vertices[1] < other.vertices[1];
			return vertices[0] < other.vertices[0];
		}
	};

	std::vector<std::shared_ptr<Shape>>
	LoopSubdivide(const Transform* ObjectToWorld, const Transform* WorldToObject,
				  bool reverseOrientation, uint32_t nLevels, uint32_t nIndices,
				  const uint32_t* vertexIndices, uint32_t nVertices, const Point3f* point);

} // pbrt
