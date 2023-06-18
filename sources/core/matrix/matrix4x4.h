#pragma once
#include"core/pbrt.h"

namespace pbrt
{
    struct Matrix4x4
    {
    public:
        Float m[4][4];

        Matrix4x4()
        {
            m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
            m[0][1] = m[0][2] = m[0][3] = m[1][0] = m[1][2] = 
            m[1][3] = m[2][0] = m[2][1] = m[2][3] = m[3][0] = 
            m[3][1] = m[3][2] = 0.0f
        }
        Matrix4x4(Float mat[4][4]);
        Matrix4x4(Float m00, Float m01, Float m02, Float m03,
                  Float m10, Float m11, Float m12, Float m13,
                  Float m20, Float m21, Float m22, Float m23,
                  Float m30, Float m31, Float m32, Float m33);
        
        //equality and inequality
        bool operator==(const Matrix4x4& other) const;
        bool operator!=(const Matrix4x4& other) const;

        //matrix operation
        friend Matrix4x4 Tranpose(const Matrix4x4& mat);
        friend Matrix4x4 Inverse(const Matrix4x4& mat);
        static Matrix4x4 Multiple(const Matrix4x4& m1, const Matrix4x4& m2);
    };
}

