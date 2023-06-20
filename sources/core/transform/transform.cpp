#include "transform.h"
#include "core/quaternion/quaternion.h"

namespace pbrt
{
    //with Gauss-Jordan elimination routine
    Matrix4x4 Inverse(const Matrix4x4& mat)
    {
		int rowFlag[4], colFlag[4];
		int flag[4] = {0, 0, 0, 0};
		//mInv copy the data from origin matrix, but also a
		Float mInv[4][4];
        memcpy(mInv, mat.m, 16 * sizeof(Float));
		//transform to upper triangle matrix
		for (int i = 0; i < 4; i++)
		{
			int row = 0, col = 0;
			Float big = 0.f;
			//choose pivot
			for (int j = 0; j < 4; j++)
			{
				if (flag[j] != 1)
				{
					for (int k = 0; k < 4; k++)
					{
                        if (flag[k] == 0 && std::abs(mInv[j][k]) >= big)
                        {
                            big = Float(std::abs(mInv[j][k]));
                            row = j;
                            col = k;
                        }
                        else if (flag[k] > 1)
                            Error("singular matrix in MatrixInvert");
					}
				}
			}
			++flag[col];
			//swap row and col for pivot
			if (row != col)
			{
				for (int k = 0; k < 4; ++k)
					std::swap(mInv[row][k], mInv[col][k]);
			}
			rowFlag[i] = row;
			colFlag[i] = col;
            if (mInv[col][col] == 0.f)
                Error("singular matrix in MatrixInvert");

			//set mInv[col][col] to one by scaling col appropriately
			Float inv = 1.f / mInv[col][col];
			mInv[col][col] = 1.f;
			for (int j = 0; j < 4; j++)
				mInv[col][j] *= inv;

			//subtract this row from others to zero out their columns
			for (int j = 0; j < 4; j++)
			{
				if (j != col)
				{
					Float save = mInv[j][col];
					mInv[j][col] = 0;
					for (int k = 0; k < 4; k++)
						mInv[j][k] -= mInv[col][k] * save;
				}
			}
		}
		//swap columns to reflect permutation
		for (int j = 3; j >= 0; j--)
		{
			if (rowFlag[j] != colFlag[j])
			{
				for (int k = 0; k < 4; k++)
					std::swap(mInv[k][rowFlag[j]], mInv[k][colFlag[j]]);
			}
		}
		return Matrix4x4(mInv);
    }

	bool Transform::IsIdentity() const
	{
		return m.m[0][0] == 1.f && m.m[0][1] == 0.f && m.m[0][2] == 0.f && m.m[0][3] == 0.f &&
			   m.m[1][0] == 0.f && m.m[1][1] == 1.f && m.m[1][2] == 0.f && m.m[1][3] == 0.f &&
			   m.m[2][0] == 0.f && m.m[2][1] == 0.f && m.m[2][2] == 1.f && m.m[2][3] == 0.f &&
			   m.m[3][0] == 0.f && m.m[3][1] == 0.f && m.m[3][2] == 0.f && m.m[3][3] == 1.f;
	}

	bool Transform::HasScale() const
	{
		Float x = this->operator()(Vector3f(1.f, 0.f, 0.f)).LengthSquared();
		Float y = this->operator()(Vector3f(0.f, 1.f, 0.f)).LengthSquared();
		Float z = this->operator()(Vector3f(0.f, 0.f, 1.f)).LengthSquared();
#define NOT_ONE(expr) ((expr) < 0.999f || (expr) > 1.001f)
		return NOT_ONE(x) || NOT_ONE(y) || NOT_ONE(z);
#undef NOT_ONE
	}

	bool Transform::IsSwapHandedness() const
	{
		Float det = m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
					m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) +
					m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
		return det < 0.f;
	}

	Transform Translate(const Vector3f& delta)
	{
		Matrix4x4 m(1.f, 0.f, 0.f, delta.x,
					0.f, 1.f, 0.f, delta.y,
					0.f, 0.f, 1.f, delta.z,
					0.f, 0.f, 0.f, 1.f);
		Matrix4x4 mInv(1.f, 0.f, 0.f, -delta.x,
					   0.f, 1.f, 0.f, -delta.y,
					   0.f, 0.f, 1.f, -delta.z,
					   0.f, 0.f, 0.f, 1.f);
		return Transform(m, mInv);
	}

	Transform Scale(Float x, Float y, Float z)
	{
		Matrix4x4 m(x, 0.f, 0.f, 0.f,
					0.f, y, 0.f, 0.f,
					0.f, 0.f, z, 0.f,
					0.f, 0.f, 0.f, 1.f);
		Matrix4x4 mInv(1.f / x, 0.f, 0.f, 0.f,
					   0.f, 1.f / y, 0.f, 0.f,
					   0.f, 0.f, 1.f / z, 0.f,
					   0.f, 0.f, 0.f, 1.f);
		return Transform(m, mInv);
	}

	Transform RotateX(Float theta)
	{
		Float sin = std::sin(Radians(theta));
		Float cos = std::cos(Radians(theta));
		Matrix4x4 m(1.f, 0.f, 0.f, 0.f,
					0.f, cos, -sin, 0.f,
					0.f, sin, cos, 0.f,
					0.f, 0.f, 0.f, 1.f);
		return Transform(m, Transpose(m));
	}

	Transform RotateY(Float theta)
	{
		Float sin = std::sin(Radians(theta));
		Float cos = std::cos(Radians(theta));
		Matrix4x4 m(cos, 0.f, sin, 0.f,
					0.f, 1.f, 0.f, 0.f,
					-sin, 0.f, cos, 0.f,
					0.f, 0.f, 0.f, 1.f);
		return Transform(m, Transpose(m));
	}

	Transform RotateZ(Float theta)
	{
		Float sin = std::sin(Radians(theta));
		Float cos = std::cos(Radians(theta));
		Matrix4x4 m(cos, -sin, 0.f, 0.f,
					sin, cos, 0.f, 0.f,
					0.f, 0.f, 1.f, 0.f,
					0.f, 0.f, 0.f, 1.f);
		return Transform(m, Transpose(m));
	}

	Transform Rotate(Float theta, const Vector3f& axis)
	{
		Vector3f a = Normalize(axis);
		Float sin = std::sin(Radians(theta));
		Float cos = std::cos(Radians(theta));
		Matrix4x4 m;
		m.m[0][0] = a.x * a.x * (1.f - cos) + cos;
		m.m[1][0] = a.x * a.y * (1.f - cos) + a.z * sin;
		m.m[2][0] = a.x * a.z * (1.f - cos) - a.y * sin;
		m.m[0][1] = a.x * a.y * (1.f - cos) - a.z * sin;
		m.m[1][1] = a.y * a.y * (1.f - cos) + cos;
		m.m[2][1] = a.y * a.z * (1.f - cos) + a.x * sin;
		m.m[0][2] = a.x * a.z * (1.f - cos) + a.y * sin;
		m.m[1][2] = a.y * a.z * (1.f - cos) - a.x * sin;
		m.m[2][2] = a.z * a.z * (1.f - cos) + cos;

		return Transform(m, Transpose(m));
	}

	Transform LookAt(const Point3f& pos, const Point3f& center, const Vector3f& worldUp)
	{
		Vector3f view = Normalize(center - pos);
		Vector3f right = Normalize(Cross(worldUp, view));
		Vector3f up = Cross(view, right);
		Matrix4x4 cameraToWorld;
		cameraToWorld.m[0][0] = right.x;
		cameraToWorld.m[1][0] = right.y;
		cameraToWorld.m[2][0] = right.z;
		cameraToWorld.m[0][1] = up.x;
		cameraToWorld.m[1][1] = up.y;
		cameraToWorld.m[2][1] = up.z;
		cameraToWorld.m[0][2] = view.x;
		cameraToWorld.m[1][2] = view.y;
		cameraToWorld.m[2][2] = view.z;
		cameraToWorld.m[0][3] = pos.x;
		cameraToWorld.m[1][3] = pos.y;
		cameraToWorld.m[2][3] = pos.z;

		Matrix4x4 worldToCamera;
		cameraToWorld.m[0][0] = right.x;
		cameraToWorld.m[0][1] = right.y;
		cameraToWorld.m[0][1] = right.z;
		cameraToWorld.m[1][0] = up.x;
		cameraToWorld.m[1][1] = up.y;
		cameraToWorld.m[1][2] = up.z;
		cameraToWorld.m[2][0] = view.x;
		cameraToWorld.m[2][1] = view.y;
		cameraToWorld.m[2][2] = view.z;
		cameraToWorld.m[3][0] = -(pos.x * right.x + pos.y * right.y + pos.z * right.z);
		cameraToWorld.m[3][1] = -(pos.x * up.x + pos.y * up.y + pos.z * up.z);
		cameraToWorld.m[3][2] = -(pos.x * view.x + pos.y * view.y + pos.z * view.z);

		return Transform(cameraToWorld, worldToCamera);
	}

	AnimatedTransform::AnimatedTransform(const Transform* startTransform, Float startTime,
										 const Transform* endTransform, Float endTime)
		: startTransform(startTransform), endTransform(endTransform), startTime(startTime),
		endTime(endTime), actuallyAnimated(*startTransform != *endTransform)
	{
		Decompose(startTransform->m, &translateStart, &rotateStart, &scalarStart);
		Decompose(endTransform->m, &translateEnd, &rotateEnd, &scalarEnd);
		//flip any one of two rotate if needed to select nearest path, here we choose rotateEnd
		if(Dot(rotateStart, rotateEnd) < 0.f)
			rotateEnd = -rotateEnd;
		hasRotation = Dot(rotateStart, rotateEnd) < One;
		//compute terms of motion derivative function
	}

	void AnimatedTransform::Decompose(const Matrix4x4& mat, Vector3f* translate,
									  Quaternion* rotate, Matrix4x4* scale)
	{
		//extract translation from transformation matrix
		translate->x = mat.m[0][3];
		translate->y = mat.m[1][3];
		translate->z = mat.m[2][3];
		//compute new transformation matrix without translation
		Matrix4x4 M = mat;
		for(uint32_t i = 0; i < 3; i++)
			M.m[i][3] = M.m[3][i] = 0.f;
		M.m[3][3] = 1.f;
		//extract rotation from transformation matrix
		Float epsilon;
		uint32_t count = 0;
		Matrix4x4 R = M;
		do
		{
			//compute next matrix
			Matrix4x4 rotateNext;
			Matrix4x4 rotateIteration = Inverse(Transpose(R));
			for(uint32_t i = 0; i < 4; i++)
			{
				for(uint32_t j = 0; j < 4; j++)
					rotateNext.m[i][j] = 0.5f * (R.m[i][j] + rotateIteration.m[i][j]);
			}
			//compute error between two rotate matrices
			epsilon = 0.f;
			for(uint32_t i = 0; i < 3; i++)
			{
				Float e = std::abs(R.m[i][0] - rotateNext.m[i][0]) +
						  std::abs(R.m[i][1] - rotateNext.m[i][1]) +
						  std::abs(R.m[i][2] - rotateNext.m[i][2]);
				epsilon += e;
			}
			R = rotateNext;
		} while(++count < 100 && epsilon > 0.0001f);
		*rotate = Quaternion(Transform(R, Transpose(R)));
		//compute scale using rotation and original matrix
		*scale = Multiple(Inverse(R), M);
	}

	void AnimatedTransform::Interpolate(Float time, Transform* transform) const
	{
		//handle boundary conditions for matrix interpolation
		if(!actuallyAnimated || time <= startTime)
		{
			*transform = *startTransform;
			return;
		}
		if(time >= endTime)
		{
			*transform = *endTransform;
			return;
		}

		Float dt = (time - startTime) / (endTime - startTime);
		Vector3f translate = (1.f - dt) * translateStart + dt * translateEnd;
		Quaternion rotate = Slerp(dt, rotateStart, rotateEnd);
		Matrix4x4 scale;
		for(uint32_t i = 0; i < 3; i++)
		{
			for(uint32_t j = 0; j < 3; j++)
				scale.m[i][j] = Lerp(dt, scalarStart.m[i][j], scalarEnd.m[i][j]);
		}
		*transform = Translate(translate) * rotate.ToTransform() * Transform(scale);
	}
}