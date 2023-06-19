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
		Transform(const Matrix4x4& mat) : m(mat), mInv(Inverse(mat)) {}
		Transform(const Matrix4x4& m, const Matrix4x4& mInv) : m(m), mInv(mInv) {}

		bool IsIdentity() const
		{
			return m.m[0][0] == 1.f && m.m[0][1] == 0.f && m.m[0][2] == 0.f && m.m[0][3] == 0.f &&
				   m.m[1][0] == 0.f && m.m[1][1] == 1.f && m.m[1][2] == 0.f && m.m[1][3] == 0.f &&
				   m.m[2][0] == 0.f && m.m[2][1] == 0.f && m.m[2][2] == 1.f && m.m[2][3] == 0.f &&
				   m.m[3][0] == 0.f && m.m[3][1] == 0.f && m.m[3][2] == 0.f && m.m[3][3] == 1.f;
		}

		bool HasScale() const;

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
			return Normal3<T>(m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z,
							  m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z,
							  m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z);
		}

	private:
		Matrix4x4 m, mInv;
	};

	//utility function
	Transform Translate(const Vector3f& delta);
	Transform Scale(Float x, Float y, Float z);

}

