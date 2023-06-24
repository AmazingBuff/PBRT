//
// Created by AmazingBuff on 2023/6/24.
//

#pragma once

#include "core/shape/shape.h"

namespace pbrt
{

	class Disk : public Shape
	{
	public:
		Disk(const Transform* ObjectToWorld, const Transform* WorldToObject, bool reverseOrientation,
			 Float height, Float radius, Float innerRadius, Float phiMax)
			: Shape(ObjectToWorld, WorldToObject, reverseOrientation),
			  height(height), radius(radius), innerRadius(innerRadius), phiMax(phiMax) {}

		Bounds3f ObjectBound() const override;
		bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
					   bool testAlphaTexture) const override;
		bool IntersectP(const Ray& ray, bool testAlphaTexture) const override;
		Float Area() const override;
	private:
		const Float height, radius, innerRadius, phiMax;
	};

} // pbrt
