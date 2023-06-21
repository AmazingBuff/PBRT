//
// Created by AmazingBuff on 2023/6/21.
//

#include "shape.h"
#include "core/transform/transform.h"
#include "core/interaction/interaction.h"

namespace pbrt
{
	Shape::Shape(const Transform* ObjectToWorld, const Transform* WorldToObject, bool reverseOrientation)
	: ObjectToWorld(ObjectToWorld), WorldToObject(WorldToObject), reverseOrientation(reverseOrientation),
	  transformSwapHandedness(ObjectToWorld->IsSwapHandedness()) {}

	Bounds3f Shape::WorldBound() const
	{
		return ObjectToWorld->operator()(ObjectBound());
	}

	bool Shape::IntersectP(const Ray& ray, bool testAlphaTexture) const
	{
		Float tHit = ray.tMax;
		SurfaceInteraction surfaceInteraction;
		return Intersect(ray, &tHit, &surfaceInteraction, testAlphaTexture);
	}

} // pbrt