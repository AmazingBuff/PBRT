#include"transform.h"

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

	bool Transform::HasScale() const
	{
		Float x = this->operator()(Vector3f(1.f, 0.f, 0.f)).LengthSquared();
		Float y = this->operator()(Vector3f(0.f, 1.f, 0.f)).LengthSquared();
		Float z = this->operator()(Vector3f(0.f, 0.f, 1.f)).LengthSquared();
#define NOT_ONE(expr) ((expr) < 0.999f || (expr) > 1.001f)
		return NOT_ONE(x) || NOT_ONE(y) || NOT_ONE(z);
#undef NOT_ONE
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
}