//
// Created by AmazingBuff on 2023/6/24.
//

#pragma once

#include "core/pbrt.h"
#include "core/geometry/geometry.h"

namespace pbrt
{

	class TriangleMesh
	{
	public:
		TriangleMesh(const Transform& ObjectToWorld, uint32_t nTriangles,
					 const uint32_t* vertexIndices, uint32_t nVertices,
					 const Point3f* position, const Vector3f* tangent,
					 const Normal3f* normal, const Point2f* uv,
					 const std::shared_ptr<Texture<Float>>& alphaMask);
	private:
		const uint32_t nTriangles, nVertices;
		std::vector<uint32_t> vertexIndices;
		std::unique_ptr<Point3f[]> position;
		std::unique_ptr<Normal3f[]> normal;
		std::unique_ptr<Vector3f[]> tangent;
		std::unique_ptr<Point2f[]> uv;
		std::shared_ptr<Texture<Float>> alphaMask;
	};

} // pbrt
