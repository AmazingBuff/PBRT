#pragma once

#include "core/pbrt.h"

namespace pbrt
{

	//vector

	template<typename T>
	class Vector3
	{
	public:
		T x, y, z;
	public:
		Vector3() { x = y = z = 0; }
		Vector3(T x, T y, T z) : x(x), y(y), z(z)
		{
			Assert(!HasNaNs());
		}

		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y) || std::isnan(z);
		}

		Float LengthSquared() const { return x * x + y * y + z * z; }
		Float Length() const { return std::sqrt(LengthSquared()); }
	public:
		T& operator[](uint32_t index)
		{
			Assert(index < 3);
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			}
		}

		const T& operator[](uint32_t index) const
		{
			Assert(index < 3);
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			}
		}

		Vector3 operator+(const Vector3& other) const
		{
			return Vector3(x + other.x, y + other.y, z + other.z);
		}

		Vector3& operator+=(const Vector3& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Vector3 operator-() const
		{
			return Vector3(-x, -y, -z);
		}

		Vector3 operator-(const Vector3& other) const
		{
			return Vector3(x - other.x, y - other.y, z - other.z);
		}

		Vector3& operator-=(const Vector3& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		Vector3 operator*(T scalar) const
		{
			return Vector3(scalar * x, scalar * y, scalar * z);
		}

		Vector3& operator*=(T scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}

		Vector3 operator/(T divisor) const
		{
			Assert(divisor != 0);
			Float inv = static_cast<Float>(1 / divisor);
			return Vector3(x * inv, y * inv, z * inv);
		}

		Vector3& operator/=(T divisor)
		{
			Assert(divisor != 0);
			Float inv = static_cast<Float>(1 / divisor);
			x *= inv;
			y *= inv;
			z *= inv;
			return *this;
		}

		bool operator==(const Vector3& other) const
		{
			return x == other.x && y == other.y && z == other.z;
		}

		bool operator!=(const Vector3& other) const
		{
			return x != other.x || y != other.y || z != other.z;
		}
	};

	template<typename T>
	inline Vector3<T> operator*(T scalar, const Vector3<T>& vector)
	{
		return vector * scalar;
	}

	template<typename T>
	inline Vector3<T> Abs(const Vector3<T>& vector)
	{
		return Vector3<T>(std::abs(vector.x), std::abs(vector.y), std::abs(vector.z));
	}

	template<typename T>
	inline T Dot(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	template<typename T>
	inline T AbsDot(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		return std::abs(Dot(v1, v2));
	}

	template<typename T>
	inline Vector3<T> Cross(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		double v1x = v1.x, v1y = v1.y, v1z = v1.z;
		double v2x = v2.x, v2y = v2.y, v2z = v2.z;
		return Vector3<T>(v1y * v2z - v1z * v2y, v1z * v2x - v1x * v2z, v1x * v2y - v1y * v2x);
	}

	template<typename T>
	inline Vector3<T> Normalize(const Vector3<T>& vector)
	{
		return vector / vector.Length();
	}

	template<typename T>
	inline T MinComponent(const Vector3<T>& vector)
	{
		return std::min(vector.x, std::min(vector.y, vector.z));
	}

	template<typename T>
	inline T MaxComponent(const Vector3<T>& vector)
	{
		return std::max(vector.x, std::max(vector.y, vector.z));
	}

	template<typename T>
	inline Vector3<T> Min(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		return Vector3<T>(std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z));
	}

	template<typename T>
	inline Vector3<T> Max(const Vector3<T>& v1, const Vector3<T>& v2)
	{
		return Vector3<T>(std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z));
	}

	template<typename T>
	inline uint32_t MaxDimension(const Vector3<T>& vector)
	{
		return (vector.x > vector.y) ? ((vector.x > vector.z) ? 0 : 2) : ((vector.y > vector.z) ? 1 : 2);
	}

	template<typename T>
	inline Vector3<T> Permute(const Vector3<T>& vector, uint32_t x, uint32_t y, uint32_t z)
	{
		return Vector3<T>(vector[x], vector[y], vector[z]);
	}

	template<typename T>
	inline void CoordinateSystem(const Vector3<T>& v1, Vector3<T>* v2, Vector3<T>* v3)
	{
		if (std::abs(v1.x) > std::abs(v1.y))
			*v2 = Vector3<T>(-v1.z, 0, v1.x) / std::sqrt(v1.x * v1.x + v1.z * v1.z);
		else
			*v2 = Vector3<T>(0, -v1.z, v1.y) / std::sqrt(v1.y * v1.y + v1.z * v1.z);
		*v3 = Cross(v1, *v2);
	}


	template<typename T>
	class Vector2
	{
	public:
		T x, y;
	public:
		Vector2() { x = y = 0; }
		Vector2(T x, T y) : x(x), y(y)
		{
			Assert(!HasNaNs());
		}

		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y);
		}
		Float LengthSquared() const { return x * x + y * y; }
		Float Length() const { return std::sqrt(LengthSquared()); }
	public:
		T& operator[](uint32_t index)
		{
			Assert(index < 2);
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			}
		}

		const T& operator[](uint32_t index) const
		{
			Assert(index < 2);
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			}
		}

		Vector2 operator+(const Vector2& other) const
		{
			return Vector3(x + other.x, y + other.y);
		}

		Vector2& operator+=(const Vector2& other)
		{
			x += other.x;
			y += other.y;
			return *this;
		}

		Vector2 operator-() const
		{
			return Vector2(-x, -y);
		}

		Vector2 operator-(const Vector2& other) const
		{
			return Vector2(x - other.x, y - other.y);
		}

		Vector2& operator-=(const Vector2& other)
		{
			x -= other.x;
			y -= other.y;
			return *this;
		}

		Vector2 operator*(T scalar) const
		{
			return Vector2(scalar * x, scalar * y);
		}

		Vector2& operator*=(T scalar)
		{
			x *= scalar;
			y *= scalar;
			return *this;
		}

		Vector2 operator/(T divisor) const
		{
			Assert(divisor != 0);
			Float inv = static_cast<Float>(1 / divisor);
			return Vector2(x * inv, y * inv);
		}

		Vector2& operator/=(T divisor)
		{
			Assert(divisor != 0);
			Float inv = static_cast<Float>(1 / divisor);
			x *= inv;
			y *= inv;
			return *this;
		}

		bool operator==(const Vector2& other) const
		{
			return x == other.x && y == other.y;
		}

		bool operator!=(const Vector2& other) const
		{
			return x != other.x || y != other.y;
		}
	};

	template<typename T>
	inline Vector2<T> operator*(T scalar, const Vector2<T>& vector)
	{
		return vector * scalar;
	}

	template<typename T>
	inline Vector2<T> Abs(const Vector2<T>& vector)
	{
		return Vector2<T>(std::abs(vector.x), std::abs(vector.y));
	}

	template<typename T>
	inline T Dot(const Vector2<T>& v1, const Vector2<T>& v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	template<typename T>
	inline T AbsDot(const Vector2<T>& v1, const Vector2<T>& v2)
	{
		return std::abs(Dot(v1, v2));
	}

	template<typename T>
	inline Vector2<T> Normalize(const Vector2<T>& vector)
	{
		return vector / vector.Length();
	}

	template<typename T>
	inline T MinComponent(const Vector2<T>& vector)
	{
		return std::min(vector.x, vector.y);
	}

	template<typename T>
	inline T MaxComponent(const Vector2<T>& vector)
	{
		return std::max(vector.x, vector.y);
	}

	template<typename T>
	inline Vector2<T> Min(const Vector2<T>& v1, const Vector2<T>& v2)
	{
		return Vector2<T>(std::min(v1.x, v2.x), std::min(v1.y, v2.y));
	}

	template<typename T>
	inline Vector2<T> Max(const Vector2<T>& v1, const Vector2<T>& v2)
	{
		return Vector2<T>(std::max(v1.x, v2.x), std::max(v1.y, v2.y));
	}

	template<typename T>
	inline Vector2<T> Permute(const Vector2<T>& vector, uint32_t x, uint32_t y)
	{
		return Vector2<T>(vector[x], vector[y]);
	}

	template<typename T>
	inline void CoordinateSystem(const Vector2<T>& v1, Vector2<T>* v2)
	{
		*v2 = Vector2<T>(-v1.y, v1.x);
	}


	using Vector2f = Vector2<Float>;
	using Vector2i = Vector2<int>;
	using Vector3f = Vector3<Float>;
	using Vector3i = Vector3<int>;



	//point

	template<typename T>
	class Point3
	{
	public:
		T x, y, z;
	public:
		Point3() { x = y = z = 0; }
		Point3(T x, T y, T z) : x(x), y(y), z(z)
		{
			Assert(!HasNaNs());
		}

		template<typename U>
		explicit Point3(const Point3<U>& point) : x(static_cast<T>(point.x)), y(static_cast<T>(point.y)), z(static_cast<T>(point.z))
		{
			Assert(!HasNaNs());
		}

		template<typename U>
		explicit operator Vector3<U>() const
		{
			return Vector3<U>(x, y, z);
		}

		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y) || std::isnan(z);
		}

		Float LengthSquared() const { return x * x + y * y + z * z; }
		Float Length() const { return std::sqrt(LengthSquared()); }
	public:
		T& operator[](uint32_t index)
		{
			Assert(index < 3);
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			}
		}

		const T& operator[](uint32_t index) const
		{
			Assert(index < 3);
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			}
		}

		Point3 operator+(const Point3<T>& other) const
		{
			return Point3(x + other.x, y + other.y, z + other.z);
		}

		Point3& operator+=(const Point3<T>& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Point3 operator+(const Vector3<T>& vector) const
		{
			return Point3(x + vector.x, y + vector.y, z + vector.z);
		}

		Point3& operator+=(const Vector3<T>& vector)
		{
			x += vector.x;
			y += vector.y;
			z += vector.z;
			return *this;
		}

		Point3 operator-() const
		{
			return Point3(-x, -y, -z);
		}

		Point3 operator-(const Vector3<T>& vector) const
		{
			return Point3(x - vector.x, y - vector.y, z - vector.z);
		}

		Point3& operator-=(const Vector3<T>& vector)
		{
			x -= vector.x;
			y -= vector.y;
			z -= vector.z;
			return *this;
		}

		Vector3<T> operator-(const Point3& other) const
		{
			return Vector3<T>(x - other.x, y - other.y, z - other.z);
		}

		Point3 operator*(T scalar) const
		{
			return Point3(scalar * x, scalar * y, scalar * z);
		}

		Point3& operator*=(T scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}

		Point3 operator/(T scalar) const
		{
			return Point3(x / scalar, y / scalar, z / scalar);
		}

		Point3& operator/=(T scalar)
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
			return *this;
		}

		bool operator==(const Point3& other) const
		{
			return x == other.x && y == other.y && z == other.z;
		}

		bool operator!=(const Point3& other) const
		{
			return x != other.x || y != other.y || z != other.z;
		}
	};

	template<typename T>
	inline Point3<T> operator*(T scalar, const Point3<T>& point)
	{
		return point * scalar;
	}

	template<typename T>
	inline Float Distance(const Point3<T>& p1, const Point3<T>& p2)
	{
		return (p1 - p2).Length();
	}

	template<typename T>
	inline Float DistanceSquared(const Point3<T>& p1, const Point3<T>& p2)
	{
		return (p1 - p2).LengthSquared();
	}

	template<typename T>
	inline Point3<T> Lerp(Float t, const Point3<T>& p0, const Point3<T>& p1)
	{
		return (1.f - t) * p0 + t * p1;
	}

	template<typename T>
	inline Point3<T> Min(const Point3<T>& p1, const Point3<T>& p2)
	{
		return Point3<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z));
	}

	template<typename T>
	inline Point3<T> Max(const Point3<T>& p1, const Point3<T>& p2)
	{
		return Point3<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z));
	}

	template<typename T>
	inline Point3<T> Floor(const Point3<T>& point)
	{
		return Point3<T>(std::floor(point.x), std::floor(point.y), std::floor(point.z));
	}

	template<typename T>
	inline Point3<T> Ceil(const Point3<T>& point)
	{
		return Point3<T>(std::ceil(point.x), std::ceil(point.y), std::ceil(point.z));
	}

	template<typename T>
	inline Point3<T> Abs(const Point3<T>& point)
	{
		return Point3<T>(std::abs(point.x), std::abs(point.y), std::abs(point.z));
	}

	template<typename T>
	inline Point3<T> Permute(const Point3<T>& point, uint32_t x, uint32_t y, uint32_t z)
	{
		return Point3<T>(point[x], point[y], point[z]);
	}


	template<typename T>
	class Point2
	{
	public:
		T x, y;
	public:
		Point2() { x = y = 0; }
		Point2(T x, T y) : x(x), y(y)
		{
			Assert(!HasNaNs());
		}
		explicit Point2(const Point3<T>& point) : x(point.x), y(point.y)
		{
			Assert(!HasNaNs());
		}

		template<typename U>
		explicit Point2(const Point2<U>& point) : x(static_cast<T>(point.x)), y(static_cast<T>(point.y))
		{
			Assert(!HasNaNs());
		}

		template<typename U>
		explicit operator Vector2<U>() const
		{
			return Vector2<U>(x, y);
		}

		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y);
		}

		Float LengthSquared() const { return x * x + y * y; }
		Float Length() const { return std::sqrt(LengthSquared()); }
	public:
		T& operator[](uint32_t index)
		{
			Assert(index < 2);
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			}
		}

		const T& operator[](uint32_t index) const
		{
			Assert(index < 2);
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			}
		}

		Point2 operator+(const Point2<T>& other) const
		{
			return Point2(x + other.x, y + other.y);
		}

		Point2& operator+=(const Point2<T>& other)
		{
			x += other.x;
			y += other.y;
			return *this;
		}

		Point2 operator+(const Vector2<T>& vector) const
		{
			return Point2(x + vector.x, y + vector.y);
		}

		Point2& operator+=(const Vector2<T>& vector)
		{
			x += vector.x;
			y += vector.y;
			return *this;
		}

		Point2 operator-() const
		{
			return Point2(-x, -y);
		}

		Point2 operator-(const Vector2<T>& vector) const
		{
			return Point2(x - vector.x, y - vector.y);
		}

		Point2& operator-=(const Vector2<T>& vector)
		{
			x -= vector.x;
			y -= vector.y;
			return *this;
		}

		Vector2<T> operator-(const Point2& other) const
		{
			return Vector2<T>(x - other.x, y - other.y);
		}

		Point2 operator*(T scalar) const
		{
			return Point2(scalar * x, scalar * y);
		}

		Point2& operator*=(T scalar)
		{
			x *= scalar;
			y *= scalar;
			return *this;
		}

		Point2 operator/(T scalar) const
		{
			return Point2(x / scalar, y / scalar);
		}

		Point2& operator/=(T scalar)
		{
			x /= scalar;
			y /= scalar;
			return *this;
		}

		bool operator==(const Point2& other) const
		{
			return x == other.x && y == other.y;
		}

		bool operator!=(const Point2& other) const
		{
			return x != other.x || y != other.y;
		}
	};

	template<typename T>
	inline Point2<T> operator*(T scalar, const Point2<T>& point)
	{
		return point * scalar;
	}

	template<typename T>
	inline Float Distance(const Point2<T>& p1, const Point2<T>& p2)
	{
		return (p1 - p2).Length();
	}

	template<typename T>
	inline Float DistanceSquared(const Point2<T>& p1, const Point2<T>& p2)
	{
		return (p1 - p2).LengthSquared();
	}

	template<typename T>
	inline Point2<T> Lerp(Float t, const Point2<T>& p0, const Point2<T>& p1)
	{
		return (1.f - t) * p0 + t * p1;
	}

	template<typename T>
	inline Point2<T> Min(const Point2<T>& p1, const Point2<T>& p2)
	{
		return Point2<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
	}

	template<typename T>
	inline Point2<T> Max(const Point2<T>& p1, const Point2<T>& p2)
	{
		return Point2<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
	}

	template<typename T>
	inline Point2<T> Floor(const Point2<T>& point)
	{
		return Point2<T>(std::floor(point.x), std::floor(point.y));
	}

	template<typename T>
	inline Point2<T> Ceil(const Point2<T>& point)
	{
		return Point2<T>(std::ceil(point.x), std::ceil(point.y));
	}

	template<typename T>
	inline Point2<T> Abs(const Point2<T>& point)
	{
		return Point2<T>(std::abs(point.x), std::abs(point.y));
	}

	template<typename T>
	inline Point2<T> Permute(const Point2<T>& point, uint32_t x, uint32_t y)
	{
		return Point2<T>(point[x], point[y]);
	}


	using Point2f = Point2<Float>;
	using Point2i = Point2<int>;
	using Point3f = Point3<Float>;
	using Point3i = Point3<int>;



	//normal
	template<typename T>
	class Normal3
	{
	public:
		T x, y, z;
	public:
		Normal3() { x = y = z = 0; }
		Normal3(T x, T y, T z) : x(x), y(y), z(z)
		{
			Assert(!HasNaNs());
		}
		explicit Normal3(const Vector3<T>& vector) : x(vector.x), y(vector.y), z(vector.z)
		{
			Assert(!HasNaNs());
		}

		bool HasNaNs() const
		{
			return std::isnan(x) || std::isnan(y) || std::isnan(z);
		}

		Float LengthSquared() const { return x * x + y * y + z * z; }
		Float Length() const { return std::sqrt(LengthSquared()); }
	public:
		T& operator[](uint32_t index)
		{
			Assert(index < 3);
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			}
		}

		const T& operator[](uint32_t index) const
		{
			Assert(index < 3);
			switch (index)
			{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			}
		}

		Normal3 operator+(const Normal3& other) const
		{
			return Normal3(x + other.x, y + other.y, z + other.z);
		}

		Normal3 operator-() const
		{
			return Normal3(-x, -y, -z);
		}

		Normal3 operator*(T scalar) const
		{
			return Normal3(scalar * x, scalar * y, scalar * z);
		}

		Normal3& operator*=(T scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}

		Normal3 operator/(T scalar) const
		{
			return Normal3(x / scalar, y / scalar, z / scalar);
		}

		bool operator==(const Normal3& other) const
		{
			return x == other.x && y == other.y && z == other.z;
		}

		bool operator!=(const Normal3& other) const
		{
			return x != other.x || y != other.y || z != other.z;
		}
	};

	template<typename T>
	inline Normal3<T> operator*(T scalar, const Normal3<T>& normal)
	{
		return normal * scalar;
	}

	template<typename T>
	inline Normal3<T> Normalize(const Normal3<T>& normal)
	{
		return normal / normal.Length();
	}

	template<typename T>
	inline T Dot(const Vector3<T>& vector, const Normal3<T>& normal)
	{
		return vector.x * normal.x + vector.y * normal.y + vector.z * normal.z;
	}

	template<typename T>
	inline T AbsDot(const Vector3<T>& vector, const Normal3<T>& normal)
	{
		return std::abs(Dot(vector, normal));
	}

	template<typename T>
	inline T Dot(const Normal3<T>& normal, const Vector3<T>& vector)
	{
		return Dot(vector, normal);
	}

	template<typename T>
	inline T AbsDot(const Normal3<T>& normal, const Vector3<T>& vector)
	{
		return std::abs(Dot(normal, vector));
	}

	template<typename T>
	inline T Dot(const Normal3<T>& n1, const Normal3<T>& n2)
	{
		return n1.x * n2.x + n1.y * n2.y + n1.z * n2.z;
	}

	template<typename T>
	inline T AbsDot(const Normal3<T>& n1, const Normal3<T>& n2)
	{
		return std::abs(Dot(n1, n2));
	}

	template<typename T>
	inline Vector3<T> Cross(const Vector3<T>& vector, const Normal3<T>& normal)
	{
		double vx = vector.x, vy = vector.y, vz = vector.z;
		double nx = normal.x, ny = normal.y, nz = normal.z;
		return Vector3<T>(vy * nz - vz * ny, vz * nx - vx * nz, vx * ny - vy * nx);
	}

	template<typename T>
	inline Vector3<T> Cross(const Normal3<T>& normal, const Vector3<T>& vector)
	{
		return Cross(vector, normal);
	}

	template<typename T>
	inline Normal3<T> Permute(const Normal3<T>& normal, uint32_t x, uint32_t y, uint32_t z)
	{
		return Normal3<T>(normal[x], normal[y], normal[z]);
	}

	template<typename T>
	inline Normal3<T> FaceForward(const Normal3<T>& normal, const Vector3<T>& vector)
	{
		return (Dot(normal, vector) < 0.f) ? -normal : normal;
	}

	template<typename T>
	inline Normal3<T> FaceForward(const Normal3<T>& n1, const Normal3<T>& n2)
	{
		return (Dot(n1, n2) < 0.f) ? -n1 : n1;
	}

	using Normal3f = Normal3<Float>;



	//ray
	class Ray
	{
	public:
		Point3f o;
		Vector3f d;
		mutable Float tMax;
		Float time;
		const std::shared_ptr<Medium> medium;
	public:
		Ray() : tMax(std::numeric_limits<Float>::max()), time(0.f), medium(nullptr) {}
		Ray(const Point3f& origin, const Vector3f& direction,
			Float tMax = std::numeric_limits<Float>::max(), Float time = 0.f, const std::shared_ptr<Medium> medium = nullptr)
			: o(origin), d(direction), tMax(tMax), time(time), medium(medium) {}

		Point3f operator()(Float t) const { return o + d * t; }
	};

	//ray differential
	class RayDifferential : public Ray
	{
	public:
		bool hasDifferentials;
		Point3f rxOrigin, ryOrigin;
		Vector3f rxDirection, ryDirection;
	public:
		RayDifferential() { hasDifferentials = false; }
		RayDifferential(const Point3f& origin, const Vector3f& direction,
			Float tMax = std::numeric_limits<Float>::max(), Float time = 0.f, const std::shared_ptr<Medium> medium = nullptr)
			: Ray(origin, direction, tMax, time, medium) {
			hasDifferentials = false;
		}
		RayDifferential(const Ray& ray) : Ray(ray) { hasDifferentials = false; }

		void ScaleDifferentials(Float s)
		{
			rxOrigin = o + (rxOrigin - o) * s;
			ryOrigin = o + (ryOrigin - o) * s;
			rxDirection = d + (rxDirection - d) * s;
			ryDirection = d + (ryDirection - d) * s;
		}
	};


	//aabb

	//with left-handed, pMin(x, y, z) --> left bottom front, pMax(x, y, z) --> right top back
	template<typename T>
	class Bounds3
	{
	public:
		Point3<T> pMin;
		Point3<T> pMax;
	public:
		Bounds3()
		{
			T minNum = std::numeric_limits<T>::lowest();
			T maxNum = std::numeric_limits<T>::max();
			pMin = Point3<T>(maxNum, maxNum, maxNum);
			pMax = Point3<T>(minNum, minNum, minNum);
		}
		Bounds3(const Point3<T>& point) : pMin(point), pMax(point) {}
		Bounds3(const Point3<T>& p1, const Point3<T>& p2)
			: pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)),
			pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z)) {}

		//0 -- left bottom front
		//1 -- right bottom front
		//2 -- left top front
		//3 -- right top front
		//4 -- left bottom back
		//5 -- right bottom back
		//6 -- left top back
		//7 -- right top back
		Point3<T> Corner(uint32_t corner) const
		{
			return Point3<T>((this->operator[](corner & 1u)).x,
				(this->operator[]((corner & 2u) ? 1 : 0)).y,
				(this->operator[]((corner & 4u) ? 1 : 0)).z);
		}

		Vector3<T> Diagonal() const { return pMax - pMin; }

		T SurfaceArea() const
		{
			Vector3<T> d = Diagonal();
			return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
		}

		T Volume() const
		{
			Vector3<T> d = Diagonal();
			return d.x * d.y * d.z;
		}

		uint32_t MaximumExtent() const
		{
			Vector3<T> d = Diagonal();
			if (d.x > d.y && d.x > d.z)
				return 0;
			else if (d.y > d.z)
				return 1;
			else
				return 2;
		}

		Point3<T> Lerp(const Point3f& t) const
		{
			return Point3<T>(pbrt::Lerp(t.x, pMin.x, pMax.x),
				pbrt::Lerp(t.y, pMin.y, pMax.y),
				pbrt::Lerp(t.z, pMin.z, pMax.z));
		}

		Vector3<T> Offset(const Point3<T>& point) const
		{
			Vector3<T> o = point - pMin;
			if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
			if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
			if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;
		}

		void BoundingSphere(Point3<T>* center, Float* radius) const
		{
			*center = (pMax + pMin) / 2;
			*radius = Inside(*center, *this) ? Distance(*center, pMax) : 0;
		}

		bool IntersectP(const Ray& ray, Float* hit_t0, Float* hit_t1) const
		{
			Float t0 = 0, t1 = ray.tMax;
			for (uint32_t i = 0; i < 3; i++)
			{
				//update interval for bounding box slab
				Float invRayDir = 1.f / ray.d[i];
				Float tNear = (pMin[i] - ray.o[i]) * invRayDir;
				Float tFar = (pMax[i] - ray.o[i]) * invRayDir;
				//update parametric interval from slab intersection t values
				if (tNear > tFar)
					std::swap(tNear, tFar);
				t0 = tNear > t0 ? tNear : t0;
				t1 = tFar < t1 ? tFar : t1;
				if (t0 > t1)
					return false;
			}
			if (hit_t0) *hit_t0 = t0;
			if (hit_t1) *hit_t1 = t1;
			return true;
		}

		bool IntersectP(const Ray& ray, const Vector3f& invDir, const uint32_t dirIsNegative[3]) const
		{
			Float txMin = (this->operator[](dirIsNegative[0]).x - ray.o.x) * invDir.x;
			Float txMax = (this->operator[](1u - dirIsNegative[0]).x - ray.o.x) * invDir.x;
			Float tyMin = (this->operator[](dirIsNegative[0]).y - ray.o.y) * invDir.y;
			Float tyMax = (this->operator[](1u - dirIsNegative[0]).y - ray.o.y) * invDir.y;

			if (txMin > tyMax || tyMin > txMax)
				return false;
			Float tMin = tyMin > txMin ? tyMin : txMin;
			Float tMax = tyMax < txMax ? tyMax : txMax;

			Float tzMin = (this->operator[](dirIsNegative[0]).z - ray.o.z) * invDir.z;
			Float tzMax = (this->operator[](1u - dirIsNegative[0]).z - ray.o.z) * invDir.z;
			if (tMin > tzMax || tzMin > tMax)
				return false;
			tMin = std::max(tMin, tzMin);
			tMax = std::min(tMax, tzMax);
			return tMin < ray.tMax && tMax > 0.f;
		}

	public:
		//0 -- pMin, 1 -- pMax
		Point3<T>& operator[](uint32_t index)
		{
			Assert(index < 2);
			switch (index)
			{
			case 0: return pMin;
			case 1: return pMax;
			}
		}
		//0 -- pMin, 1 -- pMax
		const Point3<T>& operator[](uint32_t index) const
		{
			Assert(index < 2);
			switch (index)
			{
			case 0: return pMin;
			case 1: return pMax;
			}
		}
	};

	template<typename T>
	inline Bounds3<T> Union(const Bounds3<T>& bound, const Point3<T>& point)
	{
		return Bounds3<T>(Point3<T>(std::min(bound.pMin.x, point.x),
			std::min(bound.pMin.y, point.y),
			std::min(bound.pMin.z, point.z)),
			Point3<T>(std::max(bound.pMax.x, point.x),
				std::max(bound.pMax.y, point.y),
				std::max(bound.pMax.z, point.z)));
	}

	template<typename T>
	inline Bounds3<T> Union(const Bounds3<T>& b1, const Bounds3<T>& b2)
	{
		return Bounds3<T>(Point3<T>(std::min(b1.pMin.x, b2.pMin.x),
			std::min(b1.pMin.y, b2.pMin.x),
			std::min(b1.pMin.z, b2.pMin.x)),
			Point3<T>(std::max(b1.pMax.x, b2.pMax.x),
				std::max(b1.pMax.y, b2.pMax.x),
				std::max(b1.pMax.z, b2.pMax.x)));
	}

	template<typename T>
	inline Bounds3<T> Intersect(const Bounds3<T>& b1, const Bounds3<T>& b2)
	{
		return Bounds3<T>(Point3<T>(std::max(b1.pMin.x, b2.pMin.x),
			std::max(b1.pMin.y, b2.pMin.x),
			std::max(b1.pMin.z, b2.pMin.x)),
			Point3<T>(std::min(b1.pMax.x, b2.pMax.x),
				std::min(b1.pMax.y, b2.pMax.x),
				std::min(b1.pMax.z, b2.pMax.x)));
	}

	template<typename T>
	inline bool Overlaps(const Bounds3<T>& b1, const Bounds3<T>& b2)
	{
		bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
		bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
		bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
		return x && y && z;
	}

	template<typename T>
	inline bool Inside(const Point3<T>& point, const Bounds3<T>& bound)
	{
		return point.x >= bound.pMin.x && point.x <= bound.pMax.x &&
			point.y >= bound.pMin.y && point.y <= bound.pMax.y &&
			point.z >= bound.pMin.z && point.y <= bound.pMax.z;
	}

	template<typename T>
	inline bool InsideExclusive(const Point3<T>& point, const Bounds3<T>& bound)
	{
		return point.x >= bound.pMin.x && point.x < bound.pMax.x &&
			point.y >= bound.pMin.y && point.y < bound.pMax.y &&
			point.z >= bound.pMin.z && point.y < bound.pMax.z;
	}

	template<typename T>
	inline Bounds3<T> Expand(const Bounds3<T>& bound, T delta)
	{
		return Bounds3<T>(bound.pMin - Vector3<T>(delta, delta, delta),
			bound.pMax + Vector3<T>(delta, delta, delta));
	}


	template<typename T>
	class Bounds2
	{
	public:
	};

	using Bounds2f = Bounds2<Float>;
	using Bounds2i = Bounds2<int>;
	using Bounds3f = Bounds3<Float>;
	using Bounds3i = Bounds3<int>;

} // namespace pbrt
