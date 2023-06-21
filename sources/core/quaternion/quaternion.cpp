//
// Created by AmazingBuff on 2023/6/20.
//

#include"quaternion.h"
#include "core/transform/transform.h"

namespace pbrt
{
	Quaternion::Quaternion(const Transform& transform)
	{
		//reference: rtr4 page 81
		const Matrix4x4 &m = transform.m;
		Float trace = m.m[0][0] + m.m[1][1] + m.m[2][2];
		if (trace > 0.f)
		{
			//compute w from matrix trace, then xyz
			//4w^2 = m[0][0] + m[1][1] + m[2][2] + m[3][3] (with m[3][3] == 1)
			Float s = std::sqrt(trace + 1.0f);
			w = s / 2.0f;
			s = 0.5f / s;
			v.x = (m.m[2][1] - m.m[1][2]) * s;
			v.y = (m.m[0][2] - m.m[2][0]) * s;
			v.z = (m.m[1][0] - m.m[0][1]) * s;
		}
		else
		{
			//compute largest of x, y, or z, then remaining components
			const int next[3] = {1, 2, 0};
			Float q[3];
			int i = 0;
			if (m.m[1][1] > m.m[i][i])
				i = 1;
			if (m.m[2][2] > m.m[i][i])
				i = 2;
			int j = next[i];
			int k = next[j];
			Float s = std::sqrt((m.m[i][i] - (m.m[j][j] + m.m[k][k])) + 1.0f);
			q[i] = s * 0.5f;
			if (s != 0.f)
				s = 0.5f / s;
			w = (m.m[k][j] - m.m[j][k]) * s;
			q[j] = (m.m[j][i] + m.m[i][j]) * s;
			q[k] = (m.m[k][i] + m.m[i][k]) * s;
			v.x = q[0];
			v.y = q[1];
			v.z = q[2];
		}
	}

	Transform Quaternion::ToTransform() const
	{
		Matrix4x4 m;
		m.m[0][0] = 1.f - 2.f * (v.y * v.y + v.z * v.z);
		m.m[1][0] = 2.f * (v.x * v.y + v.z * w);
		m.m[2][0] = 2.f * (v.x * v.z - v.y * w);
		m.m[0][1] = 2.f * (v.x * v.y - v.z * w);
		m.m[1][1] = 1.f - 2.f * (v.x * v.x + v.z * v.z);
		m.m[2][1] = 2.f * (v.y * v.z + v.x * w);
		m.m[0][2] = 2.f * (v.x * v.z + v.y * w);
		m.m[1][2] = 2.f * (v.y * v.z - v.x * w);
		m.m[2][2] = 1.f - 2.f * (v.x * v.x + v.y * v.y);

		return Transform(m, Transpose(m));
	}

	Quaternion Slerp(Float t, const Quaternion& q1, const Quaternion& q2)
	{
		Float cos = Dot(q1, q2);
		if(cos > One)
			return Normalize((1.f - t) * q1 + t * q2);
		else
		{
			Float theta = std::acos(Clamp(cos, -1.f, 1.f));
			Float tTheta = theta * t;
			Quaternion qPerp = Normalize(q2 - q1 * cos);
			return q1 * std::cos(tTheta) + q2 * std::sin(tTheta);
		}
	}
} // pbrt