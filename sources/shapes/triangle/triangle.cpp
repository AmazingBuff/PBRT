//
// Created by AmazingBuff on 2023/6/24.
//

#include "triangle.h"
#include "core/transform/transform.h"
#include "core/interaction/interaction.h"

namespace pbrt
{
	TriangleMesh::TriangleMesh(const Transform& ObjectToWorld, uint32_t nTriangles,
		const uint32_t* vertexIndices, uint32_t nVertices,
		const Point3f* positionIn, const Vector3f* tangentIn,
		const Normal3f* normalIn, const Point2f* uvIn,
		const std::shared_ptr<Texture<Float>>& alphaMask)
		: nTriangles(nTriangles), nVertices(nVertices),
		vertexIndices(vertexIndices, vertexIndices + 3 * nTriangles),
		alphaMask(alphaMask)
	{
		//transform mesh vertices to world space
		position.reset(new Point3f[nVertices]);
		for (uint32_t i = 0; i < nVertices; i++)
			position[i] = ObjectToWorld(positionIn[i]);

		//copy vertex data if present
	}


	Bounds3f Triangle::ObjectBound() const
	{
		const Point3f& p0 = mesh->position[vertices[0]];
		const Point3f& p1 = mesh->position[vertices[1]];
		const Point3f& p2 = mesh->position[vertices[2]];
		return Union(Bounds3f(WorldToObject->operator()(p0), WorldToObject->operator()(p1)),
			WorldToObject->operator()(p2));
	}

	Bounds3f Triangle::WorldBound() const
	{
		const Point3f& p0 = mesh->position[vertices[0]];
		const Point3f& p1 = mesh->position[vertices[1]];
		const Point3f& p2 = mesh->position[vertices[2]];
		return Union(Bounds3f(p0, p1), p2);
	}

	bool Triangle::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
		bool testAlphaTexture) const
	{
		//get triangle vertices in p0, p1, p2
		const Point3f& p0 = mesh->position[vertices[0]];
		const Point3f& p1 = mesh->position[vertices[1]];
		const Point3f& p2 = mesh->position[vertices[2]];
		//perform ray-triangle intersection test
		//transform triangle vertices to ray coordinate space
		//translate vertices based on ray origin
		Point3f p0_transform = p0 - Vector3f(ray.o);
		Point3f p1_transform = p1 - Vector3f(ray.o);
		Point3f p2_transform = p2 - Vector3f(ray.o);
		//permute components of triangle vertices and ray direction
		uint32_t kz = MaxDimension(Abs(ray.d));
		uint32_t kx = kz + 1;
		if (kx == 3)
			kx = 0;
		uint32_t ky = kx + 1;
		if (ky == 3)
			ky = 0;
		Vector3f d = Permute(ray.d, kx, ky, kz);
		p0_transform = Permute(p0_transform, kx, ky, kz);
		p1_transform = Permute(p1_transform, kx, ky, kz);
		p2_transform = Permute(p2_transform, kx, ky, kz);
		//apply shear transformation to translated vertex positions
		Float sx = -d.x / d.z;
		Float sy = -d.y / d.z;
		Float sz = 1.f / d.z;
		p0_transform.x += sx * p0_transform.z;
		p0_transform.y += sy * p0_transform.z;
		p0_transform.z *= sz;
		p1_transform.x += sx * p1_transform.z;
		p1_transform.y += sy * p1_transform.z;
		p1_transform.z *= sz;
		p2_transform.x += sx * p2_transform.z;
		p2_transform.y += sy * p2_transform.z;
		p2_transform.z *= sz;
		//compute edge function coefficients e0, e1, and e2
		Float e0 = p1_transform.x * p2_transform.y - p1_transform.y * p2_transform.x;
		Float e1 = p2_transform.x * p0_transform.y - p2_transform.y * p0_transform.x;
		Float e2 = p0_transform.x * p1_transform.y - p0_transform.y * p1_transform.x;
		//fall back to double-precision test at triangle edges

		//perform triangle edge and determinant tests
		if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
			return false;
		Float det = e0 + e1 + e2;
		if (det == 0)
			return false;
		//compute scaled hit distance to triangle and test against ray range
		Float tScaled = e0 * p0_transform.z + e1 * p1_transform.z + e2 * p2_transform.z;
		if ((det < 0 && (tScaled >= 0 || tScaled < ray.tMax * det)) ||
			(det > 0 && (tScaled <= 0 || tScaled > ray.tMax * det)))
			return false;
		//compute barycentric coordinates and t value for triangle intersection
		Float invDet = 1.f / det;
		Float b0 = e0 * invDet;
		Float b1 = e1 * invDet;
		Float b2 = e2 * invDet;
		Float t = tScaled * invDet;
		//ensure that computed triangle t is conservatively greater than zero

		//compute triangle partial derivatives
		Vector3f dp_du, dp_dv;
		std::array<Point2f, 3> uv;
		GetUVs(uv.data());
		//compute deltas for triangle partial derivatives
		Vector2f duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
		Vector3f dp02 = p0 - p2, dp12 = p1 - p2;
		Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
		if (determinant == 0.f)
		{
			//handle zero determinant for triangle partial derivative matrix
			CoordinateSystem(Normalize(Cross(p2 - p0, p1 - p0)), &dp_du, &dp_dv);
		}
		else
		{
			Float inv = 1.f / determinant;
			dp_du = (duv12[1] * dp02 - duv02[1] * dp12) * inv;
			dp_dv = (-duv12[0] * dp02 + duv02[0] * dp12) * inv;
		}
		//compute error bounds for triangle intersection

		//interpolate parametric coordinate and hit point
		Point3f pHit = b0 * p0 + b1 * p1 + b2 * p2;
		Point2f uvHit = b0 * uv[0] + b1 * uv[1] + b2 * uv[2];
		//test intersection against alpha texture, if present
		if(testAlphaTexture && mesh->alphaMask)
		{
			SurfaceInteraction surfaceInteractionLocal(pHit, Vector3f(0.f, 0.f, 0.f), uvHit, Vector3f(0.f, 0.f, 0.f),
												  dp_du, dp_dv, Normal3f(0.f, 0.f, 0.f), Normal3f(0.f, 0.f, 0.f),
												  ray.time, this);
			if(mesh->alphaMask->Evaluate(surfaceInteractionLocal) == 0.f)
				return false;
		}
		//fill in SurfaceInteraction from triangle hit
		*surfaceInteraction = SurfaceInteraction(pHit, pError, uvHit, -ray.d, dp_du, dp_dv,
											     Normal3f(0.f, 0.f, 0.f), Normal3f(0.f, 0.f, 0.f),
											     ray.time, this);
		//override surface normal for triangle
		surfaceInteraction->normal = surfaceInteraction->shading.normal = Normal3f(Normalize(Cross(dp02, dp12)));
		if(mesh->normal || mesh->tangent)
		{
			//initialize shading geometry
			Normal3f normal;
			if(mesh->normal)
				normal = Normalize(b0 * mesh->normal[vertices[0]] +
					b1 * mesh->normal[vertices[1]] +
					b2 * mesh->normal[vertices[2]]);
			else
				normal = surfaceInteraction->normal;

			Vector3f tangent;
			if(mesh->tangent)
				tangent = Normalize(b0 * mesh->tangent[vertices[0]] +
								   b1 * mesh->tangent[vertices[1]] +
								   b2 * mesh->tangent[vertices[2]]);
			else
				tangent = Normalize(surfaceInteraction->dp_du);

			Vector3f bitangent = Cross(tangent, normal);
			if(bitangent.LengthSquared() > 0.f)
			{
				bitangent = Normalize(bitangent);
				tangent = Cross(bitangent, normal);
			}
			else
				CoordinateSystem(Vector3f(normal.x, normal.y, normal.z), &tangent, &bitangent);
		}
		//ensure correct orientation of the geometric normal
		if(mesh->normal)
			surfaceInteraction->normal = FaceForward(surfaceInteraction->normal, surfaceInteraction->shading.normal);
		else if(reverseOrientation ^ transformSwapHandedness)
			surfaceInteraction->normal = surfaceInteraction->shading.normal = -surfaceInteraction->normal;

		*tHit = t;
		return false;
	}

	bool Triangle::IntersectP(const Ray& ray, bool testAlphaTexture) const
	{
		//get triangle vertices in p0, p1, p2
		const Point3f& p0 = mesh->position[vertices[0]];
		const Point3f& p1 = mesh->position[vertices[1]];
		const Point3f& p2 = mesh->position[vertices[2]];
		//perform ray-triangle intersection test
		//transform triangle vertices to ray coordinate space
		//translate vertices based on ray origin
		Point3f p0_transform = p0 - Vector3f(ray.o);
		Point3f p1_transform = p1 - Vector3f(ray.o);
		Point3f p2_transform = p2 - Vector3f(ray.o);
		//permute components of triangle vertices and ray direction
		uint32_t kz = MaxDimension(Abs(ray.d));
		uint32_t kx = kz + 1;
		if (kx == 3)
			kx = 0;
		uint32_t ky = kx + 1;
		if (ky == 3)
			ky = 0;
		Vector3f d = Permute(ray.d, kx, ky, kz);
		p0_transform = Permute(p0_transform, kx, ky, kz);
		p1_transform = Permute(p1_transform, kx, ky, kz);
		p2_transform = Permute(p2_transform, kx, ky, kz);
		//apply shear transformation to translated vertex positions
		Float sx = -d.x / d.z;
		Float sy = -d.y / d.z;
		Float sz = 1.f / d.z;
		p0_transform.x += sx * p0_transform.z;
		p0_transform.y += sy * p0_transform.z;
		p0_transform.z *= sz;
		p1_transform.x += sx * p1_transform.z;
		p1_transform.y += sy * p1_transform.z;
		p1_transform.z *= sz;
		p2_transform.x += sx * p2_transform.z;
		p2_transform.y += sy * p2_transform.z;
		p2_transform.z *= sz;
		//compute edge function coefficients e0, e1, and e2
		Float e0 = p1_transform.x * p2_transform.y - p1_transform.y * p2_transform.x;
		Float e1 = p2_transform.x * p0_transform.y - p2_transform.y * p0_transform.x;
		Float e2 = p0_transform.x * p1_transform.y - p0_transform.y * p1_transform.x;
		//fall back to double-precision test at triangle edges

		//perform triangle edge and determinant tests
		if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
			return false;
		Float det = e0 + e1 + e2;
		if (det == 0)
			return false;
		//compute scaled hit distance to triangle and test against ray range
		Float tScaled = e0 * p0_transform.z + e1 * p1_transform.z + e2 * p2_transform.z;
		if ((det < 0 && (tScaled >= 0 || tScaled < ray.tMax * det)) ||
			(det > 0 && (tScaled <= 0 || tScaled > ray.tMax * det)))
			return false;
		//compute barycentric coordinates and t value for triangle intersection
		Float invDet = 1.f / det;
		Float b0 = e0 * invDet;
		Float b1 = e1 * invDet;
		Float b2 = e2 * invDet;
		Float t = tScaled * invDet;
		//ensure that computed triangle t is conservatively greater than zero

		return true;
	}

	Float Triangle::Area() const
	{
		//get triangle vertices in p0, p1, p2
		const Point3f& p0 = mesh->position[vertices[0]];
		const Point3f& p1 = mesh->position[vertices[1]];
		const Point3f& p2 = mesh->position[vertices[2]];
		return 0.5f * Cross(p1 - p0, p2 - p0).Length();
	}

	void Triangle::GetUVs(Point2f uv[3]) const
	{
		if (mesh->uv)
		{
			uv[0] = mesh->uv[vertices[0]];
			uv[1] = mesh->uv[vertices[1]];
			uv[2] = mesh->uv[vertices[2]];
		}
		else
		{
			uv[0] = Point2f(0.f, 0.f);
			uv[1] = Point2f(1.f, 0.f);
			uv[2] = Point2f(1.f, 1.f);
		}
	}

	std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(const Transform* ObjectToWorld,
		const Transform* WorldToObject,
		bool reverseOrientation, uint32_t nTriangles,
		const uint32_t* vertexIndices, uint32_t nVertices,
		const Point3f* position, const Vector3f* tangent,
		const Normal3f* normal, const Point2f* uv,
		const std::shared_ptr<Texture<Float>>& alphaMask)
	{
		std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>(
			*ObjectToWorld, nTriangles, vertexIndices, nVertices, position, tangent, normal, uv, alphaMask);
		std::vector<std::shared_ptr<Shape>> triangles;
		for (uint32_t i = 0; i < nTriangles; i++)
			triangles.push_back(std::make_shared<Triangle>(ObjectToWorld, WorldToObject, reverseOrientation, mesh, i));

		return triangles;
	}

} // pbrt