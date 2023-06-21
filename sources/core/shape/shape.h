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
		Bounds3f WorldBound() const;
		virtual bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
							   bool testAlphaTexture) const = 0;
		virtual bool IntersectP(const Ray& ray, bool testAlphaTexture = true) const;
		virtual Float Area() const = 0;

	};

} // pbrt
