#pragma once

#include "core/pbrt.h"
#include "core/geometry/geometry.h"

namespace pbrt
{
    struct Matrix4x4
    {
    public:
        Float m[4][4];

        Matrix4x4()
        {
            m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.f;
            m[0][1] = m[0][2] = m[0][3] = m[1][0] = m[1][2] =
            m[1][3] = m[2][0] = m[2][1] = m[2][3] = m[3][0] =
            m[3][1] = m[3][2] = 0.f;
        }
        explicit Matrix4x4(Float mat[4][4])
		{
			memcpy(m, mat, 16 * sizeof(Float));
		}
        Matrix4x4(Float m00, Float m01, Float m02, Float m03,
                  Float m10, Float m11, Float m12, Float m13,
                  Float m20, Float m21, Float m22, Float m23,
                  Float m30, Float m31, Float m32, Float m33)
		{
			m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
			m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
			m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
			m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
		}
        
        //equality and inequality
        bool operator==(const Matrix4x4& other) const
		{
			for (int i = 0; i < 4; i++)
			{
				for(int j = 0; j < 4; j++)
				{
					if(m[i][j] != other.m[i][j])
						return false;
				}
			}
			return true;
		}
        bool operator!=(const Matrix4x4& other) const
		{
			for (int i = 0; i < 4; i++)
			{
				for(int j = 0; j < 4; j++)
				{
					if(m[i][j] != other.m[i][j])
						return true;
				}
			}
			return false;
		}
	};

	//matrix operation
	inline Matrix4x4 Transpose(const Matrix4x4& mat)
	{
		return Matrix4x4(mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
						 mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
						 mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
						 mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
	}
	inline Matrix4x4 Multiple(const Matrix4x4& m1, const Matrix4x4& m2)
	{
		Matrix4x4 result;
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				result.m[i][j] = m1.m[i][0] * m2.m[0][j] +
								 m1.m[i][1] * m2.m[1][j] +
								 m1.m[i][2] * m2.m[2][j] +
								 m1.m[i][3] * m2.m[3][j];
			}
		}
		return result;
	}
	Matrix4x4 Inverse(const Matrix4x4& mat);

	//transform
	class Transform
	{
	public:
		Transform() = default;
		explicit Transform(Float mat[4][4])
		{
			m = Matrix4x4(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
						  mat[1][0], mat[1][1], mat[1][2], mat[1][3],
						  mat[2][0], mat[2][1], mat[2][2], mat[2][3],
						  mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
			mInv = Inverse(m);
		}
		explicit Transform(const Matrix4x4& mat) : m(mat), mInv(Inverse(mat)) {}
		Transform(const Matrix4x4& m, const Matrix4x4& mInv) : m(m), mInv(mInv) {}

		bool IsIdentity() const;
		bool HasScale() const;
		bool IsSwapHandedness() const;

		friend Transform Inverse(const Transform& transform)
		{
			return Transform(transform.mInv, transform.m);
		}

		friend Transform Transpose(const Transform& transform)
		{
			return Transform(Transpose(transform.m), Transpose(transform.mInv));
		}

	public:
		bool operator==(const Transform& other) const
		{
			return m == other.m && mInv == other.mInv;
		}
		bool operator!=(const Transform& other) const
		{
			return m != other.m || mInv != other.mInv;
		}

		template <typename T>
		Point3<T> operator()(const Point3<T>& point) const
		{
			T x = point.x, y = point.y, z = point.z;
			T xp = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z + m.m[0][3];
			T yp = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z + m.m[1][3];
			T zp = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z + m.m[2][3];
			T wp = m.m[3][0] * x + m.m[3][1] * y + m.m[3][2] * z + m.m[3][3];
			Assert(wp != 0);
			if(wp == 1)
				return Point3<T>(xp, yp, zp);
			else
				return Point3<T>(xp, yp, zp) / wp;
		}
		template <typename T>
		Vector3<T> operator()(const Vector3<T>& vector) const
		{
			T x = vector.x, y = vector.y, z = vector.z;
			return Vector3<T>(m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z,
							  m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z,
							  m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z);
		}
		template <typename T>
		Normal3<T> operator()(const Normal3<T>& normal) const
		{
			T x = normal.x, y = normal.y, z = normal.z;
			return Normal3<T>(mInv.m[0][0] * x + mInv.m[1][0] * y + mInv.m[2][0] * z,
							  mInv.m[0][1] * x + mInv.m[1][1] * y + mInv.m[2][1] * z,
							  mInv.m[0][2] * x + mInv.m[1][2] * y + mInv.m[2][2] * z);
		}

		Ray operator()(const Ray& ray) const
		{
			Vector3f oError;
			Point3f o = this->operator()(ray.o, &oError);
			Vector3f d = this->operator()(ray.d);
			//offset ray origin to edge of error bounds and compute tMax
			return Ray(o, d, tMax, ray.time, ray.medium);
		}

		Bounds3f operator()(const Bounds3f& bound) const
		{
			Bounds3f ret(this->operator()(bound.Corner(0)));
			for(uint32_t i = 1; i < 8; i++)
				ret = Union(ret, this->operator()(bound.Corner(i)));
			return ret;
		}

		Transform operator*(const Transform& other) const
		{
			return Transform(Multiple(m, other.m), Multiple(other.mInv, mInv));
		}

	private:
		Matrix4x4 m, mInv;
		friend class Quaternion;
		friend class AnimatedTransform;
	};

	//utility function
	Transform Translate(const Vector3f& delta);
	Transform Scale(Float x, Float y, Float z);
	Transform RotateX(Float theta);
	Transform RotateY(Float theta);
	Transform RotateZ(Float theta);
	Transform Rotate(Float theta, const Vector3f& axis);
	Transform LookAt(const Point3f& pos, const Point3f& view, const Vector3f& up);



	//animated transform
	class AnimatedTransform
	{
	public:
		AnimatedTransform(const Transform* startTransform, Float startTime,
						  const Transform* endTransform, Float endTime);

		//decompose order: mat = translate * rotate * scale
		void Decompose(const Matrix4x4& mat, Vector3f* translate, Quaternion* rotate, Matrix4x4* scale);
		void Interpolate(Float time, Transform* transform) const;

	public:
		Ray operator()(const Ray& ray) const;
		RayDifferential operator()(const RayDifferential& ray) const;
		Point3f operator()(Float time, const Point3f& point) const;
		Vector3f operator()(Float time, const Vector3f& vector) const;
	private:
		const Transform* startTransform, *endTransform;
		const Float startTime, endTime;
		const bool actuallyAnimated;
		Vector3f translateStart, translateEnd;
		Quaternion rotateStart, rotateEnd;
		Matrix4x4 scalarStart, scalarEnd;
		bool hasRotation;
	};
}

