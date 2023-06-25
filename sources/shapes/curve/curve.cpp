//
// Created by AmazingBuff on 2023/6/25.
//

#include "curve.h"
#include "core/transform/transform.h"
#include "core/interaction/interaction.h"

namespace pbrt
{
	static Point3f BlossomBezier(const Point3f* point, Float u0, Float u1, Float u2)
	{
		Point3f a[3] = { Lerp(u0, point[0], point[1]), Lerp(u0, point[1], point[2]), Lerp(u0, point[2], point[3]) };
		Point3f b[2] = { Lerp(u1, a[0], a[1]), Lerp(u1, a[1], a[2]) };
		return Lerp(u2, b[0], b[1]);
	}

	static Point3f EvaluateBezier(const Point3f* point, Float u, Vector3f* derivative = nullptr)
	{
		Point3f a[3] = { Lerp(u, point[0], point[1]), Lerp(u, point[1], point[2]), Lerp(u, point[2], point[3]) };
		Point3f b[2] = { Lerp(u, a[0], a[1]), Lerp(u, a[1], a[2]) };
		if(derivative)
			*derivative = 3.f * (b[1] - b[0]);
		return Lerp(u, b[0], b[1]);
	}

	inline void SubdivideBezier(const Point3f* controlPoints, Point3f* controlPointsSplit)
	{
		controlPointsSplit[0] = controlPoints[0];
		controlPointsSplit[1] = (controlPoints[0] + controlPoints[1]) / 2.f;
		controlPointsSplit[2] = (controlPoints[0] + 2.f * controlPoints[1] + controlPoints[2]) / 4.f;
		controlPointsSplit[3] = (controlPoints[0] + 3.f * controlPoints[1] + 3.f * controlPoints[2] + controlPoints[3]) / 8.f;
		controlPointsSplit[4] = (controlPoints[1] + 2.f * controlPoints[2] + controlPoints[3]) / 4.f;
		controlPointsSplit[5] = (controlPoints[2] + controlPoints[3]) / 2.f;
		controlPointsSplit[6] = controlPoints[3];
	}

	Bounds3f Curve::ObjectBound() const
	{
		//compute object-space control points for curve segment
		std::array<Point3f, 4> controlPoints;
		controlPoints[0] = BlossomBezier(common->controlPoints.data(), uMin, uMin, uMin);
		controlPoints[1] = BlossomBezier(common->controlPoints.data(), uMin, uMin, uMax);
		controlPoints[2] = BlossomBezier(common->controlPoints.data(), uMin, uMax, uMax);
		controlPoints[3] = BlossomBezier(common->controlPoints.data(), uMax, uMax, uMax);
		Bounds3f bound = Union(Bounds3f(controlPoints[0], controlPoints[1]),
			Bounds3f(controlPoints[2], controlPoints[3]));
		std::array<Float, 2> width = { Lerp(uMin, common->widths[0], common->widths[1]),
									  Lerp(uMax, common->widths[0], common->widths[1]) };
		return Expand(bound, 0.5f * std::max(width[0], width[1]));
	}

	bool Curve::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
						  bool testAlphaTexture) const
	{
		//transform Ray to object space
		Vector3f oErr, dErr;
		Ray rayLocal = WorldToObject->operator()(ray, &oErr, &dErr);
		//compute object-space control points for curve segment
		std::array<Point3f, 4> controlPoints;
		controlPoints[0] = BlossomBezier(common->controlPoints.data(), uMin, uMin, uMin);
		controlPoints[1] = BlossomBezier(common->controlPoints.data(), uMin, uMin, uMax);
		controlPoints[2] = BlossomBezier(common->controlPoints.data(), uMin, uMax, uMax);
		controlPoints[3] = BlossomBezier(common->controlPoints.data(), uMax, uMax, uMax);
		//project curve control points to plane perpendicular to ray
		Vector3f dx, dy;
		CoordinateSystem(ray.d, &dx, &dy);
		Transform objectToRay = LookAt(ray.o, ray.o + ray.d, dx);
		std::array<Point3f, 4> controlPointsTransform = { objectToRay(controlPoints[0]), objectToRay(controlPoints[1]),
														  objectToRay(controlPoints[2]), objectToRay(controlPoints[3])};
		//compute refinement depth for curve

		return recursiveIntersect(ray, tHit, surfaceInteraction, controlPointsTransform,
								  Inverse(objectToRay), uMin, uMax, maxDepth);
	}

	bool Curve::recursiveIntersect(const Ray& ray, Float* tHit, SurfaceInteraction* surfaceInteraction,
								   const Point3f* controlPoints, const Transform& rayToObject,
								   Float u0, Float u1, uint32_t depth) const
	{
		//try to cull curves segment versus ray
		//compute bounding box of curve segment
		Bounds3f curveBounds = Union(Bounds3f(controlPoints[0], controlPoints[1]),
									 Bounds3f(controlPoints[2], controlPoints[3]));
		Float maxWidth = std::max(Lerp(u0, common->widths[0], common->widths[1]),
								  Lerp(u1, common->widths[0], common->widths[1]));
		curveBounds = Expand(curveBounds, 0.5f * maxWidth);
		//compute bounding box of ray
		Float rayLength = ray.d.Length();
		Float zMax = rayLength * ray.tMax;
		Bounds3f rayBounds(Point3f(0.f, 0.f, 0.f), Point3f(0.f, 0.f, zMax));

		if(!Overlaps(curveBounds, rayBounds))
			return false;
		if(depth > 0)
		{
			//split curve segment into sub-segments and test for intersection
			Float uMid = 0.5f * (u0 + u1);
			std::array<Point3f, 7> controlPointsSplit;
			SubdivideBezier(controlPoints, controlPointsSplit.data());
			return recursiveIntersect(ray, tHit, surfaceInteraction, controlPointsSplit.data(),
									  rayToObject, u0, uMid, depth - 1) ||
				recursiveIntersect(ray, tHit, surfaceInteraction, controlPointsSplit.data(),
								   rayToObject, u0, uMid, depth - 1);
		}
		else
		{
			//intersect ray with curve segment
			//test ray against segment endpoint boundaries
			Float edge = controlPoints[0].y * (controlPoints[0].y - controlPoints[1].y) +
						 controlPoints[0].x * (controlPoints[0].x - controlPoints[1].x);
			if(edge < 0.f)
				return false;

			edge = controlPoints[3].y * (controlPoints[2].y - controlPoints[3].y) +
				   controlPoints[3].x * (controlPoints[2].x - controlPoints[3].x);
			if(edge > 0.f)
				return false;
			//compute line that gives minimum distance to sample point
			Vector2f segmentDirection = Point2f(controlPoints[3]) - Point2f(controlPoints[0]);
			Float denominator = segmentDirection.LengthSquared();
			if(denominator == 0.f)
				return false;
			Float w = Dot(-Vector2f(Point2f(controlPoints[0])), segmentDirection) / denominator;
			//compute u coordinate of curve intersection point and hitWidth
			Float u = Clamp(Lerp(w, u0, u1), u0, u1);
			Float hitWidth = Lerp(u, common->widths[0], common->widths[1]);
			Normal3f normalHit;
			if(common->type == CurveType::Ribbon)
			{
				//scale hitWidth based on ribbon orientation
				Float sin0 = std::sin((1.f - u) * common->normalAngle) * common->invSinNormalAngle;
				Float sin1 = std::sin(u * common->normalAngle) * common->invSinNormalAngle;
				normalHit = sin0 * common->normals[0] + sin1 * common->normals[1];
				hitWidth *= AbsDot(normalHit, ray.d) / rayLength;
			}
			//test intersection point against curve width
			Vector3f dpc_dw;
			Point3f pc = EvaluateBezier(controlPoints, Clamp(w, 0.f, 1.f), &dpc_dw);
			Float pToCurveDistance2 = pc.x * pc.x + pc.y * pc.y;
			if(pToCurveDistance2 > hitWidth * hitWidth * 0.25f)
				return false;
			if(pc.z < 0.f || pc.z > zMax)
				return false;
			//compute v coordinate of curve intersection point
			Float pToCurveDistance = std::sqrt(pToCurveDistance2);
			Float edgeFunc = -pc.y * dpc_dw.x + pc.x * dpc_dw.y;
			Float v = edgeFunc > 0.f ? 0.5f + pToCurveDistance / hitWidth : 0.5f - pToCurveDistance / hitWidth;
			//compute hit t and partial derivative for curve intersection
			if(tHit != nullptr)
			{
				*tHit = pc.z / rayLength;
				//compute error bounds for curve intersection

				//compute curve derivative
				Vector3f dp_du, dp_dv;
				EvaluateBezier(common->controlPoints.data(), u, &dp_du);
				if(common->type == CurveType::Ribbon)
					dp_dv = Normalize(Cross(normalHit, dp_du)) * hitWidth;
				else
				{
					Vector3f dp_duPlane = Inverse(rayToObject)(dp_du);
					Vector3f dp_dvPlane = Normalize(Vector3f(-dp_duPlane.y, dp_duPlane.x, 0.f)) * hitWidth;
					if(common->type == CurveType::Cylinder)
					{
						//rotate to give cylindrical appearance
						Float theta = Lerp(v, -90.f, 90.f);
						Transform rotate = Rotate(-theta, dp_duPlane);//dp_duPlane is tangent
						dp_dvPlane = rotate(dp_dvPlane);
					}
					dp_dv = rayToObject(dp_dvPlane);
				}

				*surfaceInteraction = ObjectToWorld->operator()(
					SurfaceInteraction(ray(pc.z), pError, Point2f(u, v), -ray.d, dp_du, dp_dv,
									   Normal3f(0.f, 0.f, 0.f), Normal3f(0.f, 0.f, 0.f), ray.time, this));
			}
			return true;
		}
	}

} // pbrt