#include"matrix4x4.h"

namespace pbrt
{
    Matrix4x4::Matrix4x4(Float mat[4][4])
    {
        memcpy(m, mat, 16 * sizeof(Float));
    }

    Matrix4x4::Matrix4x4(Float m00, Float m01, Float m02, Float m03,
                         Float m10, Float m11, Float m12, Float m13,
                         Float m20, Float m21, Float m22, Float m23,
                         Float m30, Float m31, Float m32, Float m33)
    {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }

    bool Matrix4x4::operator==(const Matrix4x4& other) const
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
    bool Matrix4x4::operator!=(const Matrix4x4& other) const
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

    Matrix4x4 Tranpose(const Matrix4x4& mat)
    {
        return Matrix4x4(mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
                         mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
                         mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
                         mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
    }
    //with Gauss-Jordan elimination routine
    Matrix4x4 Inverse(const Matrix4x4& mat)
    {
        Float mInv[4][4];
        memcpy(mInv, mat.m, 16 * sizeof(Float));
        for(int i = 0; i < 4; i++)
        {
            int row = 0, col = 0;
            Float big = 0.0f;
            //choose pivot
        }
    }
    Matrix4x4 Matrix4x4::Multiple(const Matrix4x4& m1, const Matrix4x4& m2)
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
}