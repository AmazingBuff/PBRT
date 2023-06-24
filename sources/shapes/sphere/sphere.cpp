//
// Created by AmazingBuff on 2023/6/21.
//

#include"sphere.h"
#include"core/transform/transform.h"

namespace pbrt
{
	Bounds3f Sphere::ObjectBound() const
	{
		Point3f min, max;
		min.z = zMin;
		max.z = zMax;

		Float cosThetaMin = zMin / radius;
		Float cosThetaMax = zMax / radius;
		Float sinThetaMin = std::sqrt(1.f - cosThetaMin * cosThetaMin);
		Float sinThetaMax = std::sqrt(1.f - cosThetaMax * cosThetaMax);

		Float sinMax = std::max(sinThetaMin, sinThetaMax);
		Float sinMin = std::min(sinThetaMin, sinThetaMax);

		//zMin and zMax in different side
		if(zMin * zMax <= 0.f)
			sinMax = 1.f;

		if(phiMax < PiOver2)
		{
			min.x = radius * sinMin * std::cos(phiMax);
			min.y = 0.f;
			max.x = radius * sinMax;
			max.y = radius * sinMax * std::sin(phiMax);

		}
		else if(phiMax < Pi)
		{
			min.x = radius * sinMax * std::cos(phiMax);
			min.y = 0.f;
			max.x = radius * sinMax;
			max.y = radius * sinMax;
		}
		else if(phiMax < PiOver2 * 3.f)
		{
			min.x = -radius * sinMax;
			min.y = radius * sinMax * std::sin(phiMax);
			max.x = radius * sinMax;
			max.y = radius * sinMax;
		}
		else
		{
			min.x = -radius * sinMax;
			min.y = -radius * sinMax;
			max.x = radius * sinMax;
			max.y = radius * sinMax;
		}

		return Bounds3f(min, max);
	}

	bool Sphere::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
				   		   bool testAlphaTexture) const
	{
		Float phi;
		Point3f pHit;
		//transform ray to object space
		Vector3f oErr, dErr;
		Ray rayLocal = WorldToObject->operator()(ray, &oErr, &dErr);
		//compute quadratic sphere coefficient
		//initialize EFloat ray coordinate values
		EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
		EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);
		EFloat a = dx * dx + dy * dy + dz * dz;
		EFloat b = 2.f * (dx * ox + dy * oy + dz * oz);
		EFloat c = ox * ox + oy * oy + oz * oz - EFloat(radius) * EFloat(radius);
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
		//compute sphere hit position and phi
		pHit = rayLocal(static_cast<Float>(tShapeHit));
		//refine sphere intersection point
		if(pHit.x == 0.f && pHit.y == 0.f)
			pHit.x = 1e-5f * radius;
		phi = std::atan2(pHit.y, pHit.x);
		if(phi < 0.f)
			phi += 2.f * Pi;
		//test sphere intersection against clipping parameters
		if((zMin > -radius && pHit.z < zMin) || (zMax < radius && pHit.z > zMax) || phi > phiMax)
		{
			if(tShapeHit == t1 || t1.UpperBound() > ray.tMax)
				return false;
			tShapeHit = t1;
			//compute sphere hit position and phi
			pHit = rayLocal(static_cast<Float>(tShapeHit));
			//refine sphere intersection point
			if(pHit.x == 0.f && pHit.y == 0.f)
				pHit.x = 1e-5f * radius;
			phi = std::atan2(pHit.y, pHit.x);
			if(phi < 0.f)
				phi += 2.f * Pi;
			if((zMin > -radius && pHit.z < zMin) || (zMax < radius && pHit.z > zMax) || phi > phiMax)
				return false;
		}
		//find parametric representation of sphere hit
		Float u = phi / phiMax;
		Float theta = std::acos(Clamp(pHit.z / radius, -1.f, 1.f));
		Float v = (theta - thetaMin) / (thetaMax - thetaMin);

		Float rSinTheta = std::sqrt(pHit.x * pHit.x + pHit.y * pHit.y);
		Float invRSinTheta = 1.f / rSinTheta;
		Float cosPhi = pHit.x * invRSinTheta;
		Float sinPhi = pHit.y * invRSinTheta;
		Vector3f dp_du(-phiMax * pHit.y, phiMax * pHit.x, 0.f);
		Vector3f dp_dv = (thetaMax - thetaMin) * Vector3f(pHit.z * cosPhi, pHit.z * sinPhi, -radius * std::sin(theta));

		Vector3f d2P_duu = -phiMax * phiMax * Vector3f(pHit.x, pHit.y, 0.f);
		Vector3f d2P_duv = (thetaMax - thetaMin) * pHit.z * phiMax * Vector3f(-sinPhi, cosPhi, 0.f);
		Vector3f d2P_dvv = -(thetaMax - thetaMin) * (thetaMax - thetaMin) * Vector3f(pHit.x, pHit.y, pHit.z);
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
		//compute error bounds for sphere intersection

		//initialize SurfaceInteraction from parametric information
		*surfaceInteraction = ObjectToWorld->operator()(
			SurfaceInteraction(pHit, pError, Point2f(u, v), -ray.d, dp_du, dp_dv, dn_du, dn_dv, ray.time, this));
		//update tHit for quadratic intersection
		*tHit = static_cast<Float>(tShapeHit);
		return true;
	}

	bool Sphere::IntersectP(const Ray& ray, bool testAlphaTexture) const
	{
		Float phi;
		Point3f pHit;
		//transform ray to object space
		Vector3f oErr, dErr;
		Ray rayLocal = WorldToObject->operator()(ray, &oErr, &dErr);
		//compute quadratic sphere coefficient
		//initialize EFloat ray coordinate values
		EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
		EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);
		EFloat a = dx * dx + dy * dy + dz * dz;
		EFloat b = 2.f * (dx * ox + dy * oy + dz * oz);
		EFloat c = ox * ox + oy * oy + oz * oz - EFloat(radius) * EFloat(radius);
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
		//compute sphere hit position and phi
		pHit = rayLocal(static_cast<Float>(tShapeHit));
		//refine sphere intersection point
		if (pHit.x == 0.f && pHit.y == 0.f)
			pHit.x = 1e-5f * radius;
		phi = std::atan2(pHit.y, pHit.x);
		if (phi < 0.f)
			phi += 2.f * Pi;
		//test sphere intersection against clipping parameters
		if ((zMin > -radius && pHit.z < zMin) || (zMax < radius && pHit.z > zMax) || phi > phiMax)
		{
			if (tShapeHit == t1 || t1.UpperBound() > ray.tMax)
				return false;
			tShapeHit = t1;
			//compute sphere hit position and phi
			pHit = rayLocal(static_cast<Float>(tShapeHit));
			//refine sphere intersection point
			if (pHit.x == 0.f && pHit.y == 0.f)
				pHit.x = 1e-5f * radius;
			phi = std::atan2(pHit.y, pHit.x);
			if (phi < 0.f)
				phi += 2.f * Pi;
			if ((zMin > -radius && pHit.z < zMin) || (zMax < radius && pHit.z > zMax) || phi > phiMax)
				return false;
		}
		return true;
	}

	Float Sphere::Area() const
	{
		return phiMax * radius * (zMax - zMin);
	}
} // pbrt