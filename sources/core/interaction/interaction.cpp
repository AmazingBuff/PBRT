//
// Created by AmazingBuff on 2023/6/21.
//

#include "interaction.h"
#include "core/shape/shape.h"

namespace pbrt
{
	SurfaceInteraction::SurfaceInteraction(const Point3f& point, const Vector3f& pError,
										   const Vector3f& wo, const Point2f& uv,
										   const Vector3f& dp_du, const Vector3f& dp_dv,
										   const Normal3f& dn_du, const Normal3f& dn_dv,
										   Float time, const Shape* shape)
	: Interaction(point, Normal3f(Normalize(Cross(dp_du, dp_dv))), pError, wo, time, MediumInterface()),
	  uv(uv), dp_du(dp_du), dp_dv(dp_dv), dn_du(dn_du), dn_dv(dn_dv), shape(shape)
	{
		//adjust normal based on orientation and handedness
		if (shape && (shape->reverseOrientation ^ shape->transformSwapHandedness))
			normal = -normal;
		//initialize shading geometry from true geometry
		shading.normal = normal;
		shading.dp_du = dp_du;
		shading.dp_dv = dp_dv;
		shading.dn_du = dn_du;
		shading.dn_dv = dn_dv;
	}

	void SurfaceInteraction::SetShadingGeometry(const Vector3f& dp_du_i, const Vector3f& dp_dv_i,
												const Normal3f& dn_du_i, const Normal3f& dn_dv_i,
												bool orientationIsAuthoritative)
	{
		shading.normal = Normal3f(Normalize(Cross(dp_du_i, dp_dv_i)));
		if (shape && (shape->reverseOrientation ^ shape->transformSwapHandedness))
			shading.normal = -shading.normal;
		if(orientationIsAuthoritative)
			normal = FaceForward(normal, shading.normal);
		else
			shading.normal = FaceForward(shading.normal, normal);
		shading.dp_du = dp_du_i;
		shading.dp_dv = dp_dv_i;
		shading.dn_du = dn_du_i;
		shading.dn_dv = dn_dv_i;
	}
} // pbrt