//
// Created by AmazingBuff on 2023/6/24.
//

#include"disk.h"

namespace pbrt
{
	Bounds3f Disk::ObjectBound() const
	{
		Point3f min, max;
		min.z = max.z = height;
		max.x = radius;

		if(phiMax < PiOver2)
		{
			min.x = innerRadius * std::cos(phiMax);
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

	bool Disk::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
						 bool testAlphaTexture) const
	{
		//transform ray to object space
		Vector3f oErr, dErr;
		Ray rayLocal = WorldToObject->operator()(ray, &oErr, &dErr);
		//compute plane intersection for disk
		//reject disk intersections for rays parallel to the disk's plane
		if(rayLocal.d.z == 0.f)
			return false;
		Float tShapeHit = height - rayLocal.o.z / rayLocal.d.z;
		if(tShapeHit <= 0 || tShapeHit >= rayLocal.tMax)
			return false;
		//see if hit point is inside disk radii and phi
		Point3f pHit = rayLocal(tShapeHit);
		Float distance2 = pHit.x * pHit.x + pHit.y * pHit.y;
		if(distance2 > radius * radius || distance2 < innerRadius * innerRadius)
			return false;

		Float phi = std::atan2(pHit.y, pHit.x);
		if(phi < 0.f)
			phi += 2.f * Pi;
		if(phi > phiMax)
			return false;
		//find parametric representation of disk hit
		Float u = phi / phiMax;
		Float r = std::sqrt(distance2);
		Float v = (r - innerRadius) / (radius - innerRadius);

		Vector3f dp_du(-phiMax * pHit.y, phiMax * pHit.x, 0.f);
		Vector3f dp_dv = ((radius - innerRadius) / r) * Vector3f(pHit.x, pHit.y, 0.f);
		Normal3f dn_du(0.f, 0.f, 0.f);
		Normal3f dn_dv(0.f, 0.f, 0.f);
		//compute error bounds for disk intersection

		//initialize SurfaceInteraction from parametric information
		*surfaceInteraction = ObjectToWorld->operator()(
			SurfaceInteraction(pHit, pError, Point2f(u, v), -ray.d, dp_du, dp_dv, dn_du, dn_dv, ray.time, this));
		//update tHit for quadratic intersection
		*tHit = tShapeHit;
		return true;
	}

	bool Disk::IntersectP(const Ray& ray, bool testAlphaTexture) const
	{
		//transform ray to object space
		Vector3f oErr, dErr;
		Ray rayLocal = WorldToObject->operator()(ray, &oErr, &dErr);
		//compute plane intersection for disk
		//reject disk intersections for rays parallel to the disk's plane
		if(rayLocal.d.z == 0.f)
			return false;
		Float tShapeHit = height - rayLocal.o.z / rayLocal.d.z;
		if(tShapeHit <= 0 || tShapeHit >= rayLocal.tMax)
			return false;
		//see if hit point is inside disk radii and phi
		Point3f pHit = rayLocal(tShapeHit);
		Float distance2 = pHit.x * pHit.x + pHit.y * pHit.y;
		if(distance2 > radius * radius || distance2 < innerRadius * innerRadius)
			return false;

		Float phi = std::atan2(pHit.y, pHit.x);
		if(phi < 0.f)
			phi += 2.f * Pi;
		if(phi > phiMax)
			return false;
		return false;
	}

	Float Disk::Area() const
	{
		return 0.5f * phiMax * (radius * radius - innerRadius * innerRadius);
	}
} // pbrt