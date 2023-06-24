//
// Created by AmazingBuff on 2023/6/24.
//

#pragma once

#include "core/shape/shape.h"

namespace pbrt
{

	class Cylinder : public Shape
	{
	public:
		Cylinder(const Transform* ObjectToWorld, const Transform* WorldToObject, bool reverseOrientation,
				 Float radius, Float zMin, Float zMax, Float phiMax)
				 : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
				 radius(radius), zMin(zMin), zMax(zMax), phiMax(phiMax) {}

		Bounds3f ObjectBound() const override;
		bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
					   bool testAlphaTexture) const override;
		bool IntersectP(const Ray& ray, bool testAlphaTexture) const override;
		Float Area() const override;
	private:
		const Float radius, zMin, zMax, phiMax;
	};

} // pbrt
