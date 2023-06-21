//
// Created by AmazingBuff on 2023/6/21.
//

#pragma once
#include "core/geometry/geometry.h"

namespace pbrt
{

	class Interaction
	{
	public:
		Point3f point;
		Float time;
		Vector3f pError;
		Vector3f wo;
		Normal3f normal;
		MediumInterface mediumInterface;
	public:
		Interaction() = default;
		Interaction(const Point3f& point, const Normal3f& normal, const Vector3f& pError,
					const Vector3f& wo, Float time, const MediumInterface& mediumInterface)
					: point(point), time(time), pError(pError), wo(wo), normal(normal),
					mediumInterface(mediumInterface) {}

		bool IsSurfaceInterface() const
		{
			return normal != Normal3f();
		}
	};



	class SurfaceInteraction : public Interaction
	{
	public:
		Point2f uv;
		Vector3f dp_du, dp_dv;
		Normal3f dn_du, dn_dv;
		const Shape* shape = nullptr;
		struct
		{
			Normal3f normal;
			Vector3f dp_du, dp_dv;
			Normal3f dn_du, dn_dv;
		} shading;
	public:
		SurfaceInteraction() = default;
		SurfaceInteraction(const Point3f& point, const Vector3f& pError, const Vector3f& wo,
						   const Point2f& uv, const Vector3f& dp_du, const Vector3f& dp_dv,
						   const Normal3f& dn_du, const Normal3f& dn_dv, Float time, const Shape* shape);

		void SetShadingGeometry(const Vector3f& dp_du_i, const Vector3f& dp_dv_i, const Normal3f& dn_du_i,
								const Normal3f& dn_dv_i, bool orientationIsAuthoritative);
	};

} // pbrt
