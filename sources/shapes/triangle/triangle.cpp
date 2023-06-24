//
// Created by AmazingBuff on 2023/6/24.
//

#include "triangle.h"
#include "core/transform/transform.h"

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
		for(uint32_t i = 0; i < nVertices; i++)
			position[i] = ObjectToWorld(positionIn[i]);

		//copy vertex data if present

	}

} // pbrt