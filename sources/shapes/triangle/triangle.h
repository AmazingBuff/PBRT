//
// Created by AmazingBuff on 2023/6/24.
//

#pragma once

#include "core/shape/shape.h"

namespace pbrt
{

	class TriangleMesh
	{
	public:
		const uint32_t nTriangles, nVertices;
		std::vector<uint32_t> vertexIndices;
		std::unique_ptr<Point3f[]> position;
		std::unique_ptr<Normal3f[]> normal;
		std::unique_ptr<Vector3f[]> tangent;
		std::unique_ptr<Point2f[]> uv;
		std::shared_ptr<Texture<Float>> alphaMask;
	public:
		TriangleMesh(const Transform& ObjectToWorld, uint32_t nTriangles,
			const uint32_t* vertexIndices, uint32_t nVertices,
			const Point3f* position, const Vector3f* tangent,
			const Normal3f* normal, const Point2f* uv,
			const std::shared_ptr<Texture<Float>>& alphaMask);
	};

	class Triangle : public Shape
	{
	public:
		Triangle(const Transform* ObjectToWorld, const Transform* WorldToObject, bool reverseOrientation,
			const std::shared_ptr<TriangleMesh>& mesh, uint32_t triNumber)
			: Shape(ObjectToWorld, WorldToObject, reverseOrientation), mesh(mesh)
		{
			vertices = &mesh->vertexIndices[3 * triNumber];
		}

		Bounds3f ObjectBound() const override;
		Bounds3f WorldBound() const override;
		bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
			bool testAlphaTexture) const override;
		bool IntersectP(const Ray& ray, bool testAlphaTexture) const override;
		Float Area() const override;
	private:
		std::shared_ptr<TriangleMesh> mesh;
		const uint32_t* vertices;

	private:
		void GetUVs(Point2f uv[3]) const;
	};

	std::vector<std::shared_ptr<Shape>> CreateTriangleMesh(const Transform* ObjectToWorld,
		const Transform* WorldToObject,
		bool reverseOrientation, uint32_t nTriangles,
		const uint32_t* vertexIndices, uint32_t nVertices,
		const Point3f* position, const Vector3f* tangent,
		const Normal3f* normal, const Point2f* uv,
		const std::shared_ptr<Texture<Float>>& alphaMask);

} // pbrt
