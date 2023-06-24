//
// Created by AmazingBuff on 2023/6/24.
//

#include"cylinder.h"

namespace pbrt
{
	Bounds3f Cylinder::ObjectBound() const
	{
		Point3f min, max;
		min.z = zMin;
		max.z = zMax;
		max.x = radius;

		if(phiMax < PiOver2)
		{
			min.x = radius * std::cos(phiMax);
			min.y = 0.f;
			max.y = radius * std::sin(phiMax);
		}
		else if(phiMax < Pi)
		{
			min.x = radius * std::cos(phiMax);
			min.y = 0.f;
			max.y = radius;
		}
		else if(phiMax < PiOver2 * 3.f)
		{
			min.x = -radius;
			min.y = radius * std::sin(phiMax);
			max.y = radius;
		}
		else
		{
			min.x = -radius;
			min.y = -radius;
			max.y = radius;
		}

		return Bounds3f(min, max);
	}

	bool Cylinder::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
							 bool testAlphaTexture) const
	{
		//transform ray to object space
		Vector3f oErr, dErr;
		Ray rayLocal = WorldToObject->operator()(ray, &oErr, &dErr);
		//compute quadratic cylinder coefficient
		//initialize EFloat ray coordinate values
		EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y);
		EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y);
		EFloat a = dx * dx + dy * dy;
		EFloat b = 2.f * (dx * ox + dy * oy);
		EFloat c = ox * ox + oy * oy - EFloat(radius) * EFloat(radius);
		//solve quadratic equation for t values
		EFloat t0, t1;
		if(!Quadratic(a, b, c, &t0, &t1))
			return false;
		//check quadratic shape t0 and t1 for nearest intersection
		if(t0.UpperBound() > ray.tMax || t1.LowerBound() <= 0.f)
			return false;
		EFloat tShapeHit = t0;
		if(tShapeHit.LowerBound() <= 0.f)
		{
			tShapeHit = t1;
			if(tShapeHit.UpperBound() > ray.tMax)
				return false;
		}
		//compute cylinder hit position and phi
		Point3f pHit = rayLocal(static_cast<Float>(tShapeHit));
		//refine cylinder intersection point
		if(pHit.x == 0.f && pHit.y == 0.f)
			pHit.x = 1e-5f * radius;
		Float phi = std::atan2(pHit.y, pHit.x);
		if(phi < 0.f)
			phi += 2.f * Pi;
		//test cylinder intersection against clipping parameters
		if(pHit.z < zMin || pHit.z > zMax || phi > phiMax)
		{
			if(tShapeHit == t1 || t1.UpperBound() > ray.tMax)
				return false;
			tShapeHit = t1;
			//compute cylinder hit position and phi
			pHit = rayLocal(static_cast<Float>(tShapeHit));
			//refine cylinder intersection point
			if(pHit.x == 0.f && pHit.y == 0.f)
				pHit.x = 1e-5f * radius;
			phi = std::atan2(pHit.y, pHit.x);
			if(phi < 0.f)
				phi += 2.f * Pi;
			if(pHit.z < zMin || pHit.z > zMax || phi > phiMax)
				return false;
		}
		//find parametric representation of cylinder hit
		Float u = phi / phiMax;
		Float v = (pHit.z - zMin) / (zMax - zMin);

		Vector3f dp_du(-phiMax * pHit.y, phiMax * pHit.x, 0.f);
		Vector3f dp_dv(0.f, 0.f, zMax - zMin);

		Vector3f d2P_duu = -phiMax * phiMax * Vector3f(pHit.x, pHit.y, 0.f);
		Vector3f d2P_duv(0.f, 0.f, 0.f);
		Vector3f d2P_dvv(0.f, 0.f, 0.f);
		//compute coefficients for fundamental forms
		Float E = Dot(dp_du, dp_du);
		Float F = Dot(dp_du, dp_dv);
		Float G = Dot(dp_dv, dp_dv);
		Vector3f N = Normalize(Cross(dp_du, dp_dv));
		Float e = Dot(N, d2P_duu);
		Float f = Dot(N, d2P_duv);
		Float g = Dot(N, d2P_dvv);

		Float invEGF2 = 1.f / (E * G - F * F);
		Normal3f dn_du = Normal3f((f * F - e * G) * invEGF2 * dp_du +
								  (e * F - f * E) * invEGF2 * dp_dv);
		Normal3f dn_dv = Normal3f((g * F - f * G) * invEGF2 * dp_du +
								  (f * F - g * E) * invEGF2 * dp_dv);
		//compute error bounds for cylinder intersection

		//initialize SurfaceInteraction from parametric information
		*surfaceInteraction = ObjectToWorld->operator()(
			SurfaceInteraction(pHit, pError, Point2f(u, v), -ray.d, dp_du, dp_dv, dn_du, dn_dv, ray.time, this));
		//update tHit for quadratic intersection
		*tHit = static_cast<Float>(tShapeHit);
		return true;
	}

	bool Cylinder::IntersectP(const Ray& ray, bool testAlphaTexture) const
	{
		Float phi;
		Point3f pHit;
		//transform ray to object space
		Vector3f oErr, dErr;
		Ray rayLocal = WorldToObject->operator()(ray, &oErr, &dErr);
		//compute quadratic sphere coefficient
		//initialize EFloat ray coordinate values
		EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y);
		EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y);
		EFloat a = dx * dx + dy * dy;
		EFloat b = 2.f * (dx * ox + dy * oy);
		EFloat c = ox * ox + oy * oy - EFloat(radius) * EFloat(radius);
		//solve quadratic equation for t values
		EFloat t0, t1;
		if (!Quadratic(a, b, c, &t0, &t1))
			return false;
		//check quadratic shape t0 and t1 for nearest intersection
		if (t0.UpperBound() > ray.tMax || t1.LowerBound() <= 0.f)
			return false;
		EFloat tShapeHit = t0;
		if (tShapeHit.LowerBound() <= 0.f)
		{
			tShapeHit = t1;
			if (tShapeHit.UpperBound() > ray.tMax)
				return false;
		}
		//compute cylinder hit position and phi
		pHit = rayLocal(static_cast<Float>(tShapeHit));
		//refine cylinder intersection point
		if (pHit.x == 0.f && pHit.y == 0.f)
			pHit.x = 1e-5f * radius;
		phi = std::atan2(pHit.y, pHit.x);
		if (phi < 0.f)
			phi += 2.f * Pi;
		//test cylinder intersection against clipping parameters
		if (pHit.z < zMin || pHit.z > zMax || phi > phiMax)
		{
			if (tShapeHit == t1 || t1.UpperBound() > ray.tMax)
				return false;
			tShapeHit = t1;
			//compute cylinder hit position and phi
			pHit = rayLocal(static_cast<Float>(tShapeHit));
			//refine cylinder intersection point
			if (pHit.x == 0.f && pHit.y == 0.f)
				pHit.x = 1e-5f * radius;
			phi = std::atan2(pHit.y, pHit.x);
			if (phi < 0.f)
				phi += 2.f * Pi;
			if (pHit.z < zMin || pHit.z > zMax || phi > phiMax)
				return false;
		}
		return true;
	}

	Float Cylinder::Area() const
	{
		return (zMax - zMin) * radius * phiMax;
	}
} // pbrt