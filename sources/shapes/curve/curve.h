//
// Created by AmazingBuff on 2023/6/25.
//

#pragma once

#include "core/shape/shape.h"

namespace pbrt
{
	enum class CurveType
	{
		Flat,
		Cylinder,
		Ribbon
	};

	class CurveCommon
	{
	public:
		const CurveType type;
		const std::array<Point3f, 4> controlPoints;
		const std::array<Float, 2> widths;
		std::array<Normal3f, 2> normals;
		Float normalAngle, invSinNormalAngle;

	public:
		CurveCommon(const Point3f controlPoints[4], Float width0, Float width1, CurveType type, const Normal3f normal[2])
		: type(type), controlPoints{controlPoints[0], controlPoints[1], controlPoints[2], controlPoints[3]},
		  widths{width0, width1}
		{
			if(normal)
			{
				normals[0] = Normalize(normal[0]);
				normals[1] = Normalize(normal[1]);
				normalAngle = std::acos(Clamp(Dot(normal[0], normal[1]), 0.f, 1.f));
				invSinNormalAngle = 1.f / std::sin(normalAngle);
			}
		}
	};

	class Curve : public Shape
	{
	public:
		Curve(const Transform* ObjectToWorld, const Transform* WorldToObject, bool reverseOrientation,
			  const std::shared_ptr<CurveCommon>& common, Float uMin, Float uMax)
			  : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
			    common(common), uMin(uMin), uMax(uMax) {}


		Bounds3f ObjectBound() const override;
		bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
					   bool testAlphaTexture) const override;
		bool IntersectP(const Ray& ray, bool testAlphaTexture) const override;
		Float Area() const override;

		bool recursiveIntersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
								const Point3f* controlPoints, const Transform& rayToObject,
								Float u0, Float u1, uint32_t depth) const;
	private:
		const std::shared_ptr<CurveCommon> common;
		const Float uMin, uMax;
	};

} // pbrt
