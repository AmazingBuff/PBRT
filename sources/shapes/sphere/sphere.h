//
// Created by AmazingBuff on 2023/6/21.
//

#pragma once

#include "core/shape/shape.h"

namespace pbrt
{

	class Sphere : public Shape
	{
	public:
		Sphere(const Transform* ObjectToWorld, const Transform* WorldToObject, bool reverseOrientation,
			   Float radius, Float zMin, Float zMax, Float phiMax)
			   : Shape(ObjectToWorld, WorldToObject, reverseOrientation), radius(radius),
			   zMin(Clamp(std::min(zMin, zMax), -radius, radius)), zMax(Clamp(std::max(zMin, zMax), -radius, radius)),
			   thetaMin(std::acos(Clamp(zMin / radius, -1.f, 1.f))), thetaMax(std::acos(Clamp(zMax / radius, -1.f, 1.f))),
			   phiMax(Radians(Clamp(phiMax, 0.f, 360.f))) {}

		Bounds3f ObjectBound() const override;
		bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
					   bool testAlphaTexture) const override;
	private:
		const Float radius;
		const Float zMin, zMax;
		const Float thetaMin, thetaMax, phiMax;
	};

} // pbrt
