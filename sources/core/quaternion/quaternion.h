//
// Created by AmazingBuff on 2023/6/20.
//

#pragma once

#include "core/pbrt.h"
#include "core/geometry/geometry.h"

namespace pbrt
{

	constexpr Float One = 0.9995f;

	class Quaternion
	{
	public:
		Vector3f v;
		Float w;
	public:
		Quaternion() : v(0.f, 0.f, 0.f), w(1.f) {}
		Quaternion(const Vector3f& v, const Float w) : v(v), w(w) {}
		explicit Quaternion(const Transform& transform);

		Transform ToTransform() const;

	public:
		Quaternion operator+(const Quaternion& other) const
		{
			return Quaternion(v + other.v, w + other.w);
		}

		Quaternion& operator+=(const Quaternion& other)
		{
			v += other.v;
			w += other.w;
			return *this;
		}

		Quaternion operator-() const
		{
			return Quaternion(-v, -w);
		}

		Quaternion operator-(const Quaternion& other) const
		{
			return Quaternion(v - other.v, w - other.w);
		}

		Quaternion& operator-=(const Quaternion& other)
		{
			v -= other.v;
			w -= other.w;
			return *this;
		}

		Quaternion operator*(Float scalar) const
		{
			return Quaternion(v * scalar, w * scalar);
		}

		Quaternion& operator*=(Float scalar)
		{
			v *= scalar;
			w *= scalar;
			return *this;
		}

		Quaternion operator/(Float scalar) const
		{
			return Quaternion(v / scalar, w / scalar);
		}

		Quaternion& operator/=(Float scalar)
		{
			v /= scalar;
			w /= scalar;
			return *this;
		}
	};

	inline Quaternion operator*(Float scalar, const Quaternion& quaternion)
	{
		return quaternion * scalar;
	}

	inline Float Dot(const Quaternion& q1, const Quaternion& q2)
	{
		return Dot(q1.v, q2.v) + q1.w * q2.w;
	}

	inline Quaternion Normalize(const Quaternion& quaternion)
	{
		return quaternion / std::sqrt(Dot(quaternion, quaternion));
	}

	Quaternion Slerp(Float t, const Quaternion& q1, const Quaternion& q2);
} // pbrt
