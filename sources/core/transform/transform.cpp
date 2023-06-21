#include "transform.h"
#include "core/interaction/interaction.h"

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

	SurfaceInteraction Transform::operator()(const SurfaceInteraction& surfaceInteraction) const
	{
		SurfaceInteraction ret;
		//transform variables

		return ret;
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


	//interval definition
	class Interval
	{
	public:
		Float low, high;
	public:
		explicit Interval(Float value) : low(value), high(value) {}
		Interval(Float v0, Float v1) : low(std::min(v0, v1)), high(std::max(v0, v1)) {}

	public:
		Interval operator+(const Interval& other) const
		{
			return Interval(low + other.low, high + other.high);
		}

		Interval operator-(const Interval& other) const
		{
			return Interval(low + other.high, high - other.low);
		}

		Interval operator*(const Interval& other) const
		{
			return Interval(std::min(std::min(low * other.low, low * other.high),
									 std::min(high * other.low, high * other.high)),
							std::max(std::max(low * other.low, low * other.high),
									 std::max(high * other.low, high * other.high)));
		}
	};

	//interval must in [0, 2 * Pi]
	Interval Sin(const Interval& interval)
	{
		Float sinLow = std::sin(interval.low), sinHigh = std::sin(interval.high);
		if(sinLow > sinHigh)
			std::swap(sinLow, sinHigh);
		if(interval.low < PiOver2 && interval.high > PiOver2)
			sinHigh = 1.f;
		if(interval.low < PiOver2 * 3.f && interval.high > PiOver2 * 3.f)
			sinLow = -1.f;
		return Interval(sinLow, sinHigh);
	}

	//interval must in [0, 2 * Pi]
	Interval Cos(const Interval& interval)
	{
		Float sinLow = std::sin(interval.low), sinHigh = std::sin(interval.high);
		if(sinLow > sinHigh)
			std::swap(sinLow, sinHigh);
		if(interval.low < Pi && interval.high > Pi)
			sinLow = -1.f;
		return Interval(sinLow, sinHigh);
	}

	void IntervalFindZeros(Float c1, Float c2, Float c3, Float c4, Float c5, Float theta,
						   Interval tInterval, Float* zeros, uint32_t* zerosCount, uint32_t depth = 0)
	{
		//evaluate motion derivative in interval form:
		//da/dt = c1 + (c2 + c3 * t) * cos(2 * theta * t) + (c4 + c5 * t) * sin(2 * theta * t)
		Interval range = Interval(c1) + (Interval(c2) + Interval(c3) * tInterval) * Cos(Interval(2.f * theta) * tInterval) +
					     (Interval(c4) + Interval(c5) * tInterval) * Sin(Interval(2.f * theta) * tInterval);
		if(range.low > 0.f || range.high < 0.f || range.low == range.high)
			return;

		if(depth > 0)
		{
			//split interval and check both resulting intervals
			Float mid = (tInterval.low + tInterval.high) * 0.5f;
			IntervalFindZeros(c1, c2, c3, c4, c5, theta, Interval(tInterval.low, mid), zeros, zerosCount, depth - 1);
			IntervalFindZeros(c1, c2, c3, c4, c5, theta, Interval(mid, tInterval.high), zeros, zerosCount, depth - 1);
		}
		else
		{
			//use Newton iteration to refine zeros
			//d(da/dt) = (c3 + 2 * theta * (c4 + c5 * t)) * cos(2 * theta * t) + (c5 - 2 * theta * (c2 + c3 * t)) * sin(2 * theta * t)
			Float tNewton = (tInterval.low + tInterval.high) * 0.5f;
			for(uint32_t i = 0; i < 4; i++)
			{
				Float fNewton = c1 + (c2 + c3 * tNewton) * std::cos(2.f * theta * tNewton) +
								(c4 + c5 * tNewton) * std::sin(2.f * theta * tNewton);
				Float fPrimeNewton = (c3 + 2.f * theta * (c4 + c5 * tNewton)) * std::cos(2.f * theta * tNewton) +
									 (c5 - 2.f * theta * (c2 + c3 * tNewton)) * std::sin(2.f * theta * tNewton);
				if(fNewton == 0.f || fPrimeNewton == 0.f)
					break;
				tNewton = tNewton - fNewton / fPrimeNewton;
			}
			zeros[*zerosCount] = tNewton;
			(*zerosCount)++;
		}
	}

	AnimatedTransform::AnimatedTransform(const Transform* startTransform, Float startTime,
										 const Transform* endTransform, Float endTime)
		: startTransform(startTransform), endTransform(endTransform), startTime(startTime),
		endTime(endTime), actuallyAnimated(*startTransform != *endTransform)
	{
		Decompose(startTransform->m, &translateStart, &rotateStart, &scaleStart);
		Decompose(endTransform->m, &translateEnd, &rotateEnd, &scaleEnd);
		//flip any one of two rotate if needed to select nearest path, here we choose rotateEnd
		if(Dot(rotateStart, rotateEnd) < 0.f)
			rotateEnd = -rotateEnd;
		hasRotation = Dot(rotateStart, rotateEnd) < One;
		//compute terms of motion derivative function
		if (hasRotation)
		{
			Float cosTheta = Dot(rotateStart, rotateEnd);
			Float theta = std::acos(Clamp(cosTheta, -1.f, 1.f));
			Quaternion qPerp = Normalize(rotateEnd - rotateStart * cosTheta);

			Float t0x = translateStart.x;
			Float t0y = translateStart.y;
			Float t0z = translateStart.z;
			Float t1x = translateEnd.x;
			Float t1y = translateEnd.y;
			Float t1z = translateEnd.z;
			Float q0x = rotateStart.v.x;
			Float q0y = rotateStart.v.y;
			Float q0z = rotateStart.v.z;
			Float q0w = rotateStart.w;
			Float qPrepX = qPerp.v.x;
			Float qPrepY = qPerp.v.y;
			Float qPrepZ = qPerp.v.z;
			Float qPrepW = qPerp.w;
			Float s000 = scaleStart.m[0][0];
			Float s001 = scaleStart.m[0][1];
			Float s002 = scaleStart.m[0][2];
			Float s010 = scaleStart.m[1][0];
			Float s011 = scaleStart.m[1][1];
			Float s012 = scaleStart.m[1][2];
			Float s020 = scaleStart.m[2][0];
			Float s021 = scaleStart.m[2][1];
			Float s022 = scaleStart.m[2][2];
			Float s100 = scaleEnd.m[0][0];
			Float s101 = scaleEnd.m[0][1];
			Float s102 = scaleEnd.m[0][2];
			Float s110 = scaleEnd.m[1][0];
			Float s111 = scaleEnd.m[1][1];
			Float s112 = scaleEnd.m[1][2];
			Float s120 = scaleEnd.m[2][0];
			Float s121 = scaleEnd.m[2][1];
			Float s122 = scaleEnd.m[2][2];

			c1[0] = DerivativeTerm(
				-t0x + t1x,
				(-1 + q0y * q0y + q0z * q0z + qPrepY * qPrepY + qPrepZ * qPrepZ) *
				s000 +
				q0w * q0z * s010 - qPrepX * qPrepY * s010 +
				qPrepW * qPrepZ * s010 - q0w * q0y * s020 -
				qPrepW * qPrepY * s020 - qPrepX * qPrepZ * s020 + s100 -
				q0y * q0y * s100 - q0z * q0z * s100 - qPrepY * qPrepY * s100 -
				qPrepZ * qPrepZ * s100 - q0w * q0z * s110 +
				qPrepX * qPrepY * s110 - qPrepW * qPrepZ * s110 +
				q0w * q0y * s120 + qPrepW * qPrepY * s120 +
				qPrepX * qPrepZ * s120 +
				q0x * (-(q0y * s010) - q0z * s020 + q0y * s110 + q0z * s120),
				(-1 + q0y * q0y + q0z * q0z + qPrepY * qPrepY + qPrepZ * qPrepZ) *
				s001 +
				q0w * q0z * s011 - qPrepX * qPrepY * s011 +
				qPrepW * qPrepZ * s011 - q0w * q0y * s021 -
				qPrepW * qPrepY * s021 - qPrepX * qPrepZ * s021 + s101 -
				q0y * q0y * s101 - q0z * q0z * s101 - qPrepY * qPrepY * s101 -
				qPrepZ * qPrepZ * s101 - q0w * q0z * s111 +
				qPrepX * qPrepY * s111 - qPrepW * qPrepZ * s111 +
				q0w * q0y * s121 + qPrepW * qPrepY * s121 +
				qPrepX * qPrepZ * s121 +
				q0x * (-(q0y * s011) - q0z * s021 + q0y * s111 + q0z * s121),
				(-1 + q0y * q0y + q0z * q0z + qPrepY * qPrepY + qPrepZ * qPrepZ) *
				s002 +
				q0w * q0z * s012 - qPrepX * qPrepY * s012 +
				qPrepW * qPrepZ * s012 - q0w * q0y * s022 -
				qPrepW * qPrepY * s022 - qPrepX * qPrepZ * s022 + s102 -
				q0y * q0y * s102 - q0z * q0z * s102 - qPrepY * qPrepY * s102 -
				qPrepZ * qPrepZ * s102 - q0w * q0z * s112 +
				qPrepX * qPrepY * s112 - qPrepW * qPrepZ * s112 +
				q0w * q0y * s122 + qPrepW * qPrepY * s122 +
				qPrepX * qPrepZ * s122 +
				q0x * (-(q0y * s012) - q0z * s022 + q0y * s112 + q0z * s122));

			c2[0] = DerivativeTerm(
				0.,
				-(qPrepY * qPrepY * s000) - qPrepZ * qPrepZ * s000 +
				qPrepX * qPrepY * s010 - qPrepW * qPrepZ * s010 +
				qPrepW * qPrepY * s020 + qPrepX * qPrepZ * s020 +
				q0y * q0y * (s000 - s100) + q0z * q0z * (s000 - s100) +
				qPrepY * qPrepY * s100 + qPrepZ * qPrepZ * s100 -
				qPrepX * qPrepY * s110 + qPrepW * qPrepZ * s110 -
				qPrepW * qPrepY * s120 - qPrepX * qPrepZ * s120 +
				2 * q0x * qPrepY * s010 * theta -
				2 * q0w * qPrepZ * s010 * theta +
				2 * q0w * qPrepY * s020 * theta +
				2 * q0x * qPrepZ * s020 * theta +
				q0y *
				(q0x * (-s010 + s110) + q0w * (-s020 + s120) +
				 2 * (-2 * qPrepY * s000 + qPrepX * s010 + qPrepW * s020) *
				 theta) +
				q0z * (q0w * (s010 - s110) + q0x * (-s020 + s120) -
					   2 * (2 * qPrepZ * s000 + qPrepW * s010 - qPrepX * s020) *
					   theta),
				-(qPrepY * qPrepY * s001) - qPrepZ * qPrepZ * s001 +
				qPrepX * qPrepY * s011 - qPrepW * qPrepZ * s011 +
				qPrepW * qPrepY * s021 + qPrepX * qPrepZ * s021 +
				q0y * q0y * (s001 - s101) + q0z * q0z * (s001 - s101) +
				qPrepY * qPrepY * s101 + qPrepZ * qPrepZ * s101 -
				qPrepX * qPrepY * s111 + qPrepW * qPrepZ * s111 -
				qPrepW * qPrepY * s121 - qPrepX * qPrepZ * s121 +
				2 * q0x * qPrepY * s011 * theta -
				2 * q0w * qPrepZ * s011 * theta +
				2 * q0w * qPrepY * s021 * theta +
				2 * q0x * qPrepZ * s021 * theta +
				q0y *
				(q0x * (-s011 + s111) + q0w * (-s021 + s121) +
				 2 * (-2 * qPrepY * s001 + qPrepX * s011 + qPrepW * s021) *
				 theta) +
				q0z * (q0w * (s011 - s111) + q0x * (-s021 + s121) -
					   2 * (2 * qPrepZ * s001 + qPrepW * s011 - qPrepX * s021) *
					   theta),
				-(qPrepY * qPrepY * s002) - qPrepZ * qPrepZ * s002 +
				qPrepX * qPrepY * s012 - qPrepW * qPrepZ * s012 +
				qPrepW * qPrepY * s022 + qPrepX * qPrepZ * s022 +
				q0y * q0y * (s002 - s102) + q0z * q0z * (s002 - s102) +
				qPrepY * qPrepY * s102 + qPrepZ * qPrepZ * s102 -
				qPrepX * qPrepY * s112 + qPrepW * qPrepZ * s112 -
				qPrepW * qPrepY * s122 - qPrepX * qPrepZ * s122 +
				2 * q0x * qPrepY * s012 * theta -
				2 * q0w * qPrepZ * s012 * theta +
				2 * q0w * qPrepY * s022 * theta +
				2 * q0x * qPrepZ * s022 * theta +
				q0y *
				(q0x * (-s012 + s112) + q0w * (-s022 + s122) +
				 2 * (-2 * qPrepY * s002 + qPrepX * s012 + qPrepW * s022) *
				 theta) +
				q0z * (q0w * (s012 - s112) + q0x * (-s022 + s122) -
					   2 * (2 * qPrepZ * s002 + qPrepW * s012 - qPrepX * s022) *
					   theta));

			c3[0] = DerivativeTerm(
				0.,
				-2 * (q0x * qPrepY * s010 - q0w * qPrepZ * s010 +
					  q0w * qPrepY * s020 + q0x * qPrepZ * s020 -
					  q0x * qPrepY * s110 + q0w * qPrepZ * s110 -
					  q0w * qPrepY * s120 - q0x * qPrepZ * s120 +
					  q0y * (-2 * qPrepY * s000 + qPrepX * s010 + qPrepW * s020 +
							 2 * qPrepY * s100 - qPrepX * s110 - qPrepW * s120) +
					  q0z * (-2 * qPrepZ * s000 - qPrepW * s010 + qPrepX * s020 +
							 2 * qPrepZ * s100 + qPrepW * s110 - qPrepX * s120)) *
				theta,
				-2 * (q0x * qPrepY * s011 - q0w * qPrepZ * s011 +
					  q0w * qPrepY * s021 + q0x * qPrepZ * s021 -
					  q0x * qPrepY * s111 + q0w * qPrepZ * s111 -
					  q0w * qPrepY * s121 - q0x * qPrepZ * s121 +
					  q0y * (-2 * qPrepY * s001 + qPrepX * s011 + qPrepW * s021 +
							 2 * qPrepY * s101 - qPrepX * s111 - qPrepW * s121) +
					  q0z * (-2 * qPrepZ * s001 - qPrepW * s011 + qPrepX * s021 +
							 2 * qPrepZ * s101 + qPrepW * s111 - qPrepX * s121)) *
				theta,
				-2 * (q0x * qPrepY * s012 - q0w * qPrepZ * s012 +
					  q0w * qPrepY * s022 + q0x * qPrepZ * s022 -
					  q0x * qPrepY * s112 + q0w * qPrepZ * s112 -
					  q0w * qPrepY * s122 - q0x * qPrepZ * s122 +
					  q0y * (-2 * qPrepY * s002 + qPrepX * s012 + qPrepW * s022 +
							 2 * qPrepY * s102 - qPrepX * s112 - qPrepW * s122) +
					  q0z * (-2 * qPrepZ * s002 - qPrepW * s012 + qPrepX * s022 +
							 2 * qPrepZ * s102 + qPrepW * s112 - qPrepX * s122)) *
				theta);

			c4[0] = DerivativeTerm(
				0.,
				-(q0x * qPrepY * s010) + q0w * qPrepZ * s010 - q0w * qPrepY * s020 -
				q0x * qPrepZ * s020 + q0x * qPrepY * s110 -
				q0w * qPrepZ * s110 + q0w * qPrepY * s120 +
				q0x * qPrepZ * s120 + 2 * q0y * q0y * s000 * theta +
				2 * q0z * q0z * s000 * theta -
				2 * qPrepY * qPrepY * s000 * theta -
				2 * qPrepZ * qPrepZ * s000 * theta +
				2 * qPrepX * qPrepY * s010 * theta -
				2 * qPrepW * qPrepZ * s010 * theta +
				2 * qPrepW * qPrepY * s020 * theta +
				2 * qPrepX * qPrepZ * s020 * theta +
				q0y * (-(qPrepX * s010) - qPrepW * s020 +
					   2 * qPrepY * (s000 - s100) + qPrepX * s110 +
					   qPrepW * s120 - 2 * q0x * s010 * theta -
					   2 * q0w * s020 * theta) +
				q0z * (2 * qPrepZ * s000 + qPrepW * s010 - qPrepX * s020 -
					   2 * qPrepZ * s100 - qPrepW * s110 + qPrepX * s120 +
					   2 * q0w * s010 * theta - 2 * q0x * s020 * theta),
				-(q0x * qPrepY * s011) + q0w * qPrepZ * s011 - q0w * qPrepY * s021 -
				q0x * qPrepZ * s021 + q0x * qPrepY * s111 -
				q0w * qPrepZ * s111 + q0w * qPrepY * s121 +
				q0x * qPrepZ * s121 + 2 * q0y * q0y * s001 * theta +
				2 * q0z * q0z * s001 * theta -
				2 * qPrepY * qPrepY * s001 * theta -
				2 * qPrepZ * qPrepZ * s001 * theta +
				2 * qPrepX * qPrepY * s011 * theta -
				2 * qPrepW * qPrepZ * s011 * theta +
				2 * qPrepW * qPrepY * s021 * theta +
				2 * qPrepX * qPrepZ * s021 * theta +
				q0y * (-(qPrepX * s011) - qPrepW * s021 +
					   2 * qPrepY * (s001 - s101) + qPrepX * s111 +
					   qPrepW * s121 - 2 * q0x * s011 * theta -
					   2 * q0w * s021 * theta) +
				q0z * (2 * qPrepZ * s001 + qPrepW * s011 - qPrepX * s021 -
					   2 * qPrepZ * s101 - qPrepW * s111 + qPrepX * s121 +
					   2 * q0w * s011 * theta - 2 * q0x * s021 * theta),
				-(q0x * qPrepY * s012) + q0w * qPrepZ * s012 - q0w * qPrepY * s022 -
				q0x * qPrepZ * s022 + q0x * qPrepY * s112 -
				q0w * qPrepZ * s112 + q0w * qPrepY * s122 +
				q0x * qPrepZ * s122 + 2 * q0y * q0y * s002 * theta +
				2 * q0z * q0z * s002 * theta -
				2 * qPrepY * qPrepY * s002 * theta -
				2 * qPrepZ * qPrepZ * s002 * theta +
				2 * qPrepX * qPrepY * s012 * theta -
				2 * qPrepW * qPrepZ * s012 * theta +
				2 * qPrepW * qPrepY * s022 * theta +
				2 * qPrepX * qPrepZ * s022 * theta +
				q0y * (-(qPrepX * s012) - qPrepW * s022 +
					   2 * qPrepY * (s002 - s102) + qPrepX * s112 +
					   qPrepW * s122 - 2 * q0x * s012 * theta -
					   2 * q0w * s022 * theta) +
				q0z * (2 * qPrepZ * s002 + qPrepW * s012 - qPrepX * s022 -
					   2 * qPrepZ * s102 - qPrepW * s112 + qPrepX * s122 +
					   2 * q0w * s012 * theta - 2 * q0x * s022 * theta));

			c5[0] = DerivativeTerm(
				0.,
				2 * (qPrepY * qPrepY * s000 + qPrepZ * qPrepZ * s000 -
					 qPrepX * qPrepY * s010 + qPrepW * qPrepZ * s010 -
					 qPrepW * qPrepY * s020 - qPrepX * qPrepZ * s020 -
					 qPrepY * qPrepY * s100 - qPrepZ * qPrepZ * s100 +
					 q0y * q0y * (-s000 + s100) + q0z * q0z * (-s000 + s100) +
					 qPrepX * qPrepY * s110 - qPrepW * qPrepZ * s110 +
					 q0y * (q0x * (s010 - s110) + q0w * (s020 - s120)) +
					 qPrepW * qPrepY * s120 + qPrepX * qPrepZ * s120 +
					 q0z * (-(q0w * s010) + q0x * s020 + q0w * s110 - q0x * s120)) *
				theta,
				2 * (qPrepY * qPrepY * s001 + qPrepZ * qPrepZ * s001 -
					 qPrepX * qPrepY * s011 + qPrepW * qPrepZ * s011 -
					 qPrepW * qPrepY * s021 - qPrepX * qPrepZ * s021 -
					 qPrepY * qPrepY * s101 - qPrepZ * qPrepZ * s101 +
					 q0y * q0y * (-s001 + s101) + q0z * q0z * (-s001 + s101) +
					 qPrepX * qPrepY * s111 - qPrepW * qPrepZ * s111 +
					 q0y * (q0x * (s011 - s111) + q0w * (s021 - s121)) +
					 qPrepW * qPrepY * s121 + qPrepX * qPrepZ * s121 +
					 q0z * (-(q0w * s011) + q0x * s021 + q0w * s111 - q0x * s121)) *
				theta,
				2 * (qPrepY * qPrepY * s002 + qPrepZ * qPrepZ * s002 -
					 qPrepX * qPrepY * s012 + qPrepW * qPrepZ * s012 -
					 qPrepW * qPrepY * s022 - qPrepX * qPrepZ * s022 -
					 qPrepY * qPrepY * s102 - qPrepZ * qPrepZ * s102 +
					 q0y * q0y * (-s002 + s102) + q0z * q0z * (-s002 + s102) +
					 qPrepX * qPrepY * s112 - qPrepW * qPrepZ * s112 +
					 q0y * (q0x * (s012 - s112) + q0w * (s022 - s122)) +
					 qPrepW * qPrepY * s122 + qPrepX * qPrepZ * s122 +
					 q0z * (-(q0w * s012) + q0x * s022 + q0w * s112 - q0x * s122)) *
				theta);

			c1[1] = DerivativeTerm(
				-t0y + t1y,
				-(qPrepX * qPrepY * s000) - qPrepW * qPrepZ * s000 - s010 +
				q0z * q0z * s010 + qPrepX * qPrepX * s010 +
				qPrepZ * qPrepZ * s010 - q0y * q0z * s020 +
				qPrepW * qPrepX * s020 - qPrepY * qPrepZ * s020 +
				qPrepX * qPrepY * s100 + qPrepW * qPrepZ * s100 +
				q0w * q0z * (-s000 + s100) + q0x * q0x * (s010 - s110) + s110 -
				q0z * q0z * s110 - qPrepX * qPrepX * s110 -
				qPrepZ * qPrepZ * s110 +
				q0x * (q0y * (-s000 + s100) + q0w * (s020 - s120)) +
				q0y * q0z * s120 - qPrepW * qPrepX * s120 +
				qPrepY * qPrepZ * s120,
				-(qPrepX * qPrepY * s001) - qPrepW * qPrepZ * s001 - s011 +
				q0z * q0z * s011 + qPrepX * qPrepX * s011 +
				qPrepZ * qPrepZ * s011 - q0y * q0z * s021 +
				qPrepW * qPrepX * s021 - qPrepY * qPrepZ * s021 +
				qPrepX * qPrepY * s101 + qPrepW * qPrepZ * s101 +
				q0w * q0z * (-s001 + s101) + q0x * q0x * (s011 - s111) + s111 -
				q0z * q0z * s111 - qPrepX * qPrepX * s111 -
				qPrepZ * qPrepZ * s111 +
				q0x * (q0y * (-s001 + s101) + q0w * (s021 - s121)) +
				q0y * q0z * s121 - qPrepW * qPrepX * s121 +
				qPrepY * qPrepZ * s121,
				-(qPrepX * qPrepY * s002) - qPrepW * qPrepZ * s002 - s012 +
				q0z * q0z * s012 + qPrepX * qPrepX * s012 +
				qPrepZ * qPrepZ * s012 - q0y * q0z * s022 +
				qPrepW * qPrepX * s022 - qPrepY * qPrepZ * s022 +
				qPrepX * qPrepY * s102 + qPrepW * qPrepZ * s102 +
				q0w * q0z * (-s002 + s102) + q0x * q0x * (s012 - s112) + s112 -
				q0z * q0z * s112 - qPrepX * qPrepX * s112 -
				qPrepZ * qPrepZ * s112 +
				q0x * (q0y * (-s002 + s102) + q0w * (s022 - s122)) +
				q0y * q0z * s122 - qPrepW * qPrepX * s122 +
				qPrepY * qPrepZ * s122);

			c2[1] = DerivativeTerm(
				0.,
				qPrepX * qPrepY * s000 + qPrepW * qPrepZ * s000 + q0z * q0z * s010 -
				qPrepX * qPrepX * s010 - qPrepZ * qPrepZ * s010 -
				q0y * q0z * s020 - qPrepW * qPrepX * s020 +
				qPrepY * qPrepZ * s020 - qPrepX * qPrepY * s100 -
				qPrepW * qPrepZ * s100 + q0x * q0x * (s010 - s110) -
				q0z * q0z * s110 + qPrepX * qPrepX * s110 +
				qPrepZ * qPrepZ * s110 + q0y * q0z * s120 +
				qPrepW * qPrepX * s120 - qPrepY * qPrepZ * s120 +
				2 * q0z * qPrepW * s000 * theta +
				2 * q0y * qPrepX * s000 * theta -
				4 * q0z * qPrepZ * s010 * theta +
				2 * q0z * qPrepY * s020 * theta +
				2 * q0y * qPrepZ * s020 * theta +
				q0x * (q0w * s020 + q0y * (-s000 + s100) - q0w * s120 +
					   2 * qPrepY * s000 * theta - 4 * qPrepX * s010 * theta -
					   2 * qPrepW * s020 * theta) +
				q0w * (-(q0z * s000) + q0z * s100 + 2 * qPrepZ * s000 * theta -
					   2 * qPrepX * s020 * theta),
				qPrepX * qPrepY * s001 + qPrepW * qPrepZ * s001 + q0z * q0z * s011 -
				qPrepX * qPrepX * s011 - qPrepZ * qPrepZ * s011 -
				q0y * q0z * s021 - qPrepW * qPrepX * s021 +
				qPrepY * qPrepZ * s021 - qPrepX * qPrepY * s101 -
				qPrepW * qPrepZ * s101 + q0x * q0x * (s011 - s111) -
				q0z * q0z * s111 + qPrepX * qPrepX * s111 +
				qPrepZ * qPrepZ * s111 + q0y * q0z * s121 +
				qPrepW * qPrepX * s121 - qPrepY * qPrepZ * s121 +
				2 * q0z * qPrepW * s001 * theta +
				2 * q0y * qPrepX * s001 * theta -
				4 * q0z * qPrepZ * s011 * theta +
				2 * q0z * qPrepY * s021 * theta +
				2 * q0y * qPrepZ * s021 * theta +
				q0x * (q0w * s021 + q0y * (-s001 + s101) - q0w * s121 +
					   2 * qPrepY * s001 * theta - 4 * qPrepX * s011 * theta -
					   2 * qPrepW * s021 * theta) +
				q0w * (-(q0z * s001) + q0z * s101 + 2 * qPrepZ * s001 * theta -
					   2 * qPrepX * s021 * theta),
				qPrepX * qPrepY * s002 + qPrepW * qPrepZ * s002 + q0z * q0z * s012 -
				qPrepX * qPrepX * s012 - qPrepZ * qPrepZ * s012 -
				q0y * q0z * s022 - qPrepW * qPrepX * s022 +
				qPrepY * qPrepZ * s022 - qPrepX * qPrepY * s102 -
				qPrepW * qPrepZ * s102 + q0x * q0x * (s012 - s112) -
				q0z * q0z * s112 + qPrepX * qPrepX * s112 +
				qPrepZ * qPrepZ * s112 + q0y * q0z * s122 +
				qPrepW * qPrepX * s122 - qPrepY * qPrepZ * s122 +
				2 * q0z * qPrepW * s002 * theta +
				2 * q0y * qPrepX * s002 * theta -
				4 * q0z * qPrepZ * s012 * theta +
				2 * q0z * qPrepY * s022 * theta +
				2 * q0y * qPrepZ * s022 * theta +
				q0x * (q0w * s022 + q0y * (-s002 + s102) - q0w * s122 +
					   2 * qPrepY * s002 * theta - 4 * qPrepX * s012 * theta -
					   2 * qPrepW * s022 * theta) +
				q0w * (-(q0z * s002) + q0z * s102 + 2 * qPrepZ * s002 * theta -
					   2 * qPrepX * s022 * theta));

			c3[1] = DerivativeTerm(
				0., 2 * (-(q0x * qPrepY * s000) - q0w * qPrepZ * s000 +
						 2 * q0x * qPrepX * s010 + q0x * qPrepW * s020 +
						 q0w * qPrepX * s020 + q0x * qPrepY * s100 +
						 q0w * qPrepZ * s100 - 2 * q0x * qPrepX * s110 -
						 q0x * qPrepW * s120 - q0w * qPrepX * s120 +
						 q0z * (2 * qPrepZ * s010 - qPrepY * s020 +
								qPrepW * (-s000 + s100) - 2 * qPrepZ * s110 +
								qPrepY * s120) +
						 q0y * (-(qPrepX * s000) - qPrepZ * s020 + qPrepX * s100 +
								qPrepZ * s120)) *
					theta,
				2 * (-(q0x * qPrepY * s001) - q0w * qPrepZ * s001 +
					 2 * q0x * qPrepX * s011 + q0x * qPrepW * s021 +
					 q0w * qPrepX * s021 + q0x * qPrepY * s101 +
					 q0w * qPrepZ * s101 - 2 * q0x * qPrepX * s111 -
					 q0x * qPrepW * s121 - q0w * qPrepX * s121 +
					 q0z * (2 * qPrepZ * s011 - qPrepY * s021 +
							qPrepW * (-s001 + s101) - 2 * qPrepZ * s111 +
							qPrepY * s121) +
					 q0y * (-(qPrepX * s001) - qPrepZ * s021 + qPrepX * s101 +
							qPrepZ * s121)) *
				theta,
				2 * (-(q0x * qPrepY * s002) - q0w * qPrepZ * s002 +
					 2 * q0x * qPrepX * s012 + q0x * qPrepW * s022 +
					 q0w * qPrepX * s022 + q0x * qPrepY * s102 +
					 q0w * qPrepZ * s102 - 2 * q0x * qPrepX * s112 -
					 q0x * qPrepW * s122 - q0w * qPrepX * s122 +
					 q0z * (2 * qPrepZ * s012 - qPrepY * s022 +
							qPrepW * (-s002 + s102) - 2 * qPrepZ * s112 +
							qPrepY * s122) +
					 q0y * (-(qPrepX * s002) - qPrepZ * s022 + qPrepX * s102 +
							qPrepZ * s122)) *
				theta);

			c4[1] = DerivativeTerm(
				0.,
				-(q0x * qPrepY * s000) - q0w * qPrepZ * s000 +
				2 * q0x * qPrepX * s010 + q0x * qPrepW * s020 +
				q0w * qPrepX * s020 + q0x * qPrepY * s100 +
				q0w * qPrepZ * s100 - 2 * q0x * qPrepX * s110 -
				q0x * qPrepW * s120 - q0w * qPrepX * s120 +
				2 * qPrepX * qPrepY * s000 * theta +
				2 * qPrepW * qPrepZ * s000 * theta +
				2 * q0x * q0x * s010 * theta + 2 * q0z * q0z * s010 * theta -
				2 * qPrepX * qPrepX * s010 * theta -
				2 * qPrepZ * qPrepZ * s010 * theta +
				2 * q0w * q0x * s020 * theta -
				2 * qPrepW * qPrepX * s020 * theta +
				2 * qPrepY * qPrepZ * s020 * theta +
				q0y * (-(qPrepX * s000) - qPrepZ * s020 + qPrepX * s100 +
					   qPrepZ * s120 - 2 * q0x * s000 * theta) +
				q0z * (2 * qPrepZ * s010 - qPrepY * s020 +
					   qPrepW * (-s000 + s100) - 2 * qPrepZ * s110 +
					   qPrepY * s120 - 2 * q0w * s000 * theta -
					   2 * q0y * s020 * theta),
				-(q0x * qPrepY * s001) - q0w * qPrepZ * s001 +
				2 * q0x * qPrepX * s011 + q0x * qPrepW * s021 +
				q0w * qPrepX * s021 + q0x * qPrepY * s101 +
				q0w * qPrepZ * s101 - 2 * q0x * qPrepX * s111 -
				q0x * qPrepW * s121 - q0w * qPrepX * s121 +
				2 * qPrepX * qPrepY * s001 * theta +
				2 * qPrepW * qPrepZ * s001 * theta +
				2 * q0x * q0x * s011 * theta + 2 * q0z * q0z * s011 * theta -
				2 * qPrepX * qPrepX * s011 * theta -
				2 * qPrepZ * qPrepZ * s011 * theta +
				2 * q0w * q0x * s021 * theta -
				2 * qPrepW * qPrepX * s021 * theta +
				2 * qPrepY * qPrepZ * s021 * theta +
				q0y * (-(qPrepX * s001) - qPrepZ * s021 + qPrepX * s101 +
					   qPrepZ * s121 - 2 * q0x * s001 * theta) +
				q0z * (2 * qPrepZ * s011 - qPrepY * s021 +
					   qPrepW * (-s001 + s101) - 2 * qPrepZ * s111 +
					   qPrepY * s121 - 2 * q0w * s001 * theta -
					   2 * q0y * s021 * theta),
				-(q0x * qPrepY * s002) - q0w * qPrepZ * s002 +
				2 * q0x * qPrepX * s012 + q0x * qPrepW * s022 +
				q0w * qPrepX * s022 + q0x * qPrepY * s102 +
				q0w * qPrepZ * s102 - 2 * q0x * qPrepX * s112 -
				q0x * qPrepW * s122 - q0w * qPrepX * s122 +
				2 * qPrepX * qPrepY * s002 * theta +
				2 * qPrepW * qPrepZ * s002 * theta +
				2 * q0x * q0x * s012 * theta + 2 * q0z * q0z * s012 * theta -
				2 * qPrepX * qPrepX * s012 * theta -
				2 * qPrepZ * qPrepZ * s012 * theta +
				2 * q0w * q0x * s022 * theta -
				2 * qPrepW * qPrepX * s022 * theta +
				2 * qPrepY * qPrepZ * s022 * theta +
				q0y * (-(qPrepX * s002) - qPrepZ * s022 + qPrepX * s102 +
					   qPrepZ * s122 - 2 * q0x * s002 * theta) +
				q0z * (2 * qPrepZ * s012 - qPrepY * s022 +
					   qPrepW * (-s002 + s102) - 2 * qPrepZ * s112 +
					   qPrepY * s122 - 2 * q0w * s002 * theta -
					   2 * q0y * s022 * theta));

			c5[1] = DerivativeTerm(
				0., -2 * (qPrepX * qPrepY * s000 + qPrepW * qPrepZ * s000 +
						  q0z * q0z * s010 - qPrepX * qPrepX * s010 -
						  qPrepZ * qPrepZ * s010 - q0y * q0z * s020 -
						  qPrepW * qPrepX * s020 + qPrepY * qPrepZ * s020 -
						  qPrepX * qPrepY * s100 - qPrepW * qPrepZ * s100 +
						  q0w * q0z * (-s000 + s100) + q0x * q0x * (s010 - s110) -
						  q0z * q0z * s110 + qPrepX * qPrepX * s110 +
						  qPrepZ * qPrepZ * s110 +
						  q0x * (q0y * (-s000 + s100) + q0w * (s020 - s120)) +
						  q0y * q0z * s120 + qPrepW * qPrepX * s120 -
						  qPrepY * qPrepZ * s120) *
					theta,
				-2 * (qPrepX * qPrepY * s001 + qPrepW * qPrepZ * s001 +
					  q0z * q0z * s011 - qPrepX * qPrepX * s011 -
					  qPrepZ * qPrepZ * s011 - q0y * q0z * s021 -
					  qPrepW * qPrepX * s021 + qPrepY * qPrepZ * s021 -
					  qPrepX * qPrepY * s101 - qPrepW * qPrepZ * s101 +
					  q0w * q0z * (-s001 + s101) + q0x * q0x * (s011 - s111) -
					  q0z * q0z * s111 + qPrepX * qPrepX * s111 +
					  qPrepZ * qPrepZ * s111 +
					  q0x * (q0y * (-s001 + s101) + q0w * (s021 - s121)) +
					  q0y * q0z * s121 + qPrepW * qPrepX * s121 -
					  qPrepY * qPrepZ * s121) *
				theta,
				-2 * (qPrepX * qPrepY * s002 + qPrepW * qPrepZ * s002 +
					  q0z * q0z * s012 - qPrepX * qPrepX * s012 -
					  qPrepZ * qPrepZ * s012 - q0y * q0z * s022 -
					  qPrepW * qPrepX * s022 + qPrepY * qPrepZ * s022 -
					  qPrepX * qPrepY * s102 - qPrepW * qPrepZ * s102 +
					  q0w * q0z * (-s002 + s102) + q0x * q0x * (s012 - s112) -
					  q0z * q0z * s112 + qPrepX * qPrepX * s112 +
					  qPrepZ * qPrepZ * s112 +
					  q0x * (q0y * (-s002 + s102) + q0w * (s022 - s122)) +
					  q0y * q0z * s122 + qPrepW * qPrepX * s122 -
					  qPrepY * qPrepZ * s122) *
				theta);

			c1[2] = DerivativeTerm(
				-t0z + t1z, (qPrepW * qPrepY * s000 - qPrepX * qPrepZ * s000 -
							 q0y * q0z * s010 - qPrepW * qPrepX * s010 -
							 qPrepY * qPrepZ * s010 - s020 + q0y * q0y * s020 +
							 qPrepX * qPrepX * s020 + qPrepY * qPrepY * s020 -
							 qPrepW * qPrepY * s100 + qPrepX * qPrepZ * s100 +
							 q0x * q0z * (-s000 + s100) + q0y * q0z * s110 +
							 qPrepW * qPrepX * s110 + qPrepY * qPrepZ * s110 +
							 q0w * (q0y * (s000 - s100) + q0x * (-s010 + s110)) +
							 q0x * q0x * (s020 - s120) + s120 - q0y * q0y * s120 -
							 qPrepX * qPrepX * s120 - qPrepY * qPrepY * s120),
				(qPrepW * qPrepY * s001 - qPrepX * qPrepZ * s001 -
				 q0y * q0z * s011 - qPrepW * qPrepX * s011 -
				 qPrepY * qPrepZ * s011 - s021 + q0y * q0y * s021 +
				 qPrepX * qPrepX * s021 + qPrepY * qPrepY * s021 -
				 qPrepW * qPrepY * s101 + qPrepX * qPrepZ * s101 +
				 q0x * q0z * (-s001 + s101) + q0y * q0z * s111 +
				 qPrepW * qPrepX * s111 + qPrepY * qPrepZ * s111 +
				 q0w * (q0y * (s001 - s101) + q0x * (-s011 + s111)) +
				 q0x * q0x * (s021 - s121) + s121 - q0y * q0y * s121 -
				 qPrepX * qPrepX * s121 - qPrepY * qPrepY * s121),
				(qPrepW * qPrepY * s002 - qPrepX * qPrepZ * s002 -
				 q0y * q0z * s012 - qPrepW * qPrepX * s012 -
				 qPrepY * qPrepZ * s012 - s022 + q0y * q0y * s022 +
				 qPrepX * qPrepX * s022 + qPrepY * qPrepY * s022 -
				 qPrepW * qPrepY * s102 + qPrepX * qPrepZ * s102 +
				 q0x * q0z * (-s002 + s102) + q0y * q0z * s112 +
				 qPrepW * qPrepX * s112 + qPrepY * qPrepZ * s112 +
				 q0w * (q0y * (s002 - s102) + q0x * (-s012 + s112)) +
				 q0x * q0x * (s022 - s122) + s122 - q0y * q0y * s122 -
				 qPrepX * qPrepX * s122 - qPrepY * qPrepY * s122));

			c2[2] = DerivativeTerm(
				0.,
				(q0w * q0y * s000 - q0x * q0z * s000 - qPrepW * qPrepY * s000 +
				 qPrepX * qPrepZ * s000 - q0w * q0x * s010 - q0y * q0z * s010 +
				 qPrepW * qPrepX * s010 + qPrepY * qPrepZ * s010 +
				 q0x * q0x * s020 + q0y * q0y * s020 - qPrepX * qPrepX * s020 -
				 qPrepY * qPrepY * s020 - q0w * q0y * s100 + q0x * q0z * s100 +
				 qPrepW * qPrepY * s100 - qPrepX * qPrepZ * s100 +
				 q0w * q0x * s110 + q0y * q0z * s110 - qPrepW * qPrepX * s110 -
				 qPrepY * qPrepZ * s110 - q0x * q0x * s120 - q0y * q0y * s120 +
				 qPrepX * qPrepX * s120 + qPrepY * qPrepY * s120 -
				 2 * q0y * qPrepW * s000 * theta + 2 * q0z * qPrepX * s000 * theta -
				 2 * q0w * qPrepY * s000 * theta + 2 * q0x * qPrepZ * s000 * theta +
				 2 * q0x * qPrepW * s010 * theta + 2 * q0w * qPrepX * s010 * theta +
				 2 * q0z * qPrepY * s010 * theta + 2 * q0y * qPrepZ * s010 * theta -
				 4 * q0x * qPrepX * s020 * theta - 4 * q0y * qPrepY * s020 * theta),
				(q0w * q0y * s001 - q0x * q0z * s001 - qPrepW * qPrepY * s001 +
				 qPrepX * qPrepZ * s001 - q0w * q0x * s011 - q0y * q0z * s011 +
				 qPrepW * qPrepX * s011 + qPrepY * qPrepZ * s011 +
				 q0x * q0x * s021 + q0y * q0y * s021 - qPrepX * qPrepX * s021 -
				 qPrepY * qPrepY * s021 - q0w * q0y * s101 + q0x * q0z * s101 +
				 qPrepW * qPrepY * s101 - qPrepX * qPrepZ * s101 +
				 q0w * q0x * s111 + q0y * q0z * s111 - qPrepW * qPrepX * s111 -
				 qPrepY * qPrepZ * s111 - q0x * q0x * s121 - q0y * q0y * s121 +
				 qPrepX * qPrepX * s121 + qPrepY * qPrepY * s121 -
				 2 * q0y * qPrepW * s001 * theta + 2 * q0z * qPrepX * s001 * theta -
				 2 * q0w * qPrepY * s001 * theta + 2 * q0x * qPrepZ * s001 * theta +
				 2 * q0x * qPrepW * s011 * theta + 2 * q0w * qPrepX * s011 * theta +
				 2 * q0z * qPrepY * s011 * theta + 2 * q0y * qPrepZ * s011 * theta -
				 4 * q0x * qPrepX * s021 * theta - 4 * q0y * qPrepY * s021 * theta),
				(q0w * q0y * s002 - q0x * q0z * s002 - qPrepW * qPrepY * s002 +
				 qPrepX * qPrepZ * s002 - q0w * q0x * s012 - q0y * q0z * s012 +
				 qPrepW * qPrepX * s012 + qPrepY * qPrepZ * s012 +
				 q0x * q0x * s022 + q0y * q0y * s022 - qPrepX * qPrepX * s022 -
				 qPrepY * qPrepY * s022 - q0w * q0y * s102 + q0x * q0z * s102 +
				 qPrepW * qPrepY * s102 - qPrepX * qPrepZ * s102 +
				 q0w * q0x * s112 + q0y * q0z * s112 - qPrepW * qPrepX * s112 -
				 qPrepY * qPrepZ * s112 - q0x * q0x * s122 - q0y * q0y * s122 +
				 qPrepX * qPrepX * s122 + qPrepY * qPrepY * s122 -
				 2 * q0y * qPrepW * s002 * theta + 2 * q0z * qPrepX * s002 * theta -
				 2 * q0w * qPrepY * s002 * theta + 2 * q0x * qPrepZ * s002 * theta +
				 2 * q0x * qPrepW * s012 * theta + 2 * q0w * qPrepX * s012 * theta +
				 2 * q0z * qPrepY * s012 * theta + 2 * q0y * qPrepZ * s012 * theta -
				 4 * q0x * qPrepX * s022 * theta -
				 4 * q0y * qPrepY * s022 * theta));

			c3[2] = DerivativeTerm(
				0., -2 * (-(q0w * qPrepY * s000) + q0x * qPrepZ * s000 +
						  q0x * qPrepW * s010 + q0w * qPrepX * s010 -
						  2 * q0x * qPrepX * s020 + q0w * qPrepY * s100 -
						  q0x * qPrepZ * s100 - q0x * qPrepW * s110 -
						  q0w * qPrepX * s110 +
						  q0z * (qPrepX * s000 + qPrepY * s010 - qPrepX * s100 -
								 qPrepY * s110) +
						  2 * q0x * qPrepX * s120 +
						  q0y * (qPrepZ * s010 - 2 * qPrepY * s020 +
								 qPrepW * (-s000 + s100) - qPrepZ * s110 +
								 2 * qPrepY * s120)) *
					theta,
				-2 * (-(q0w * qPrepY * s001) + q0x * qPrepZ * s001 +
					  q0x * qPrepW * s011 + q0w * qPrepX * s011 -
					  2 * q0x * qPrepX * s021 + q0w * qPrepY * s101 -
					  q0x * qPrepZ * s101 - q0x * qPrepW * s111 -
					  q0w * qPrepX * s111 +
					  q0z * (qPrepX * s001 + qPrepY * s011 - qPrepX * s101 -
							 qPrepY * s111) +
					  2 * q0x * qPrepX * s121 +
					  q0y * (qPrepZ * s011 - 2 * qPrepY * s021 +
							 qPrepW * (-s001 + s101) - qPrepZ * s111 +
							 2 * qPrepY * s121)) *
				theta,
				-2 * (-(q0w * qPrepY * s002) + q0x * qPrepZ * s002 +
					  q0x * qPrepW * s012 + q0w * qPrepX * s012 -
					  2 * q0x * qPrepX * s022 + q0w * qPrepY * s102 -
					  q0x * qPrepZ * s102 - q0x * qPrepW * s112 -
					  q0w * qPrepX * s112 +
					  q0z * (qPrepX * s002 + qPrepY * s012 - qPrepX * s102 -
							 qPrepY * s112) +
					  2 * q0x * qPrepX * s122 +
					  q0y * (qPrepZ * s012 - 2 * qPrepY * s022 +
							 qPrepW * (-s002 + s102) - qPrepZ * s112 +
							 2 * qPrepY * s122)) *
				theta);

			c4[2] = DerivativeTerm(
				0.,
				q0w * qPrepY * s000 - q0x * qPrepZ * s000 - q0x * qPrepW * s010 -
				q0w * qPrepX * s010 + 2 * q0x * qPrepX * s020 -
				q0w * qPrepY * s100 + q0x * qPrepZ * s100 +
				q0x * qPrepW * s110 + q0w * qPrepX * s110 -
				2 * q0x * qPrepX * s120 - 2 * qPrepW * qPrepY * s000 * theta +
				2 * qPrepX * qPrepZ * s000 * theta -
				2 * q0w * q0x * s010 * theta +
				2 * qPrepW * qPrepX * s010 * theta +
				2 * qPrepY * qPrepZ * s010 * theta +
				2 * q0x * q0x * s020 * theta + 2 * q0y * q0y * s020 * theta -
				2 * qPrepX * qPrepX * s020 * theta -
				2 * qPrepY * qPrepY * s020 * theta +
				q0z * (-(qPrepX * s000) - qPrepY * s010 + qPrepX * s100 +
					   qPrepY * s110 - 2 * q0x * s000 * theta) +
				q0y * (-(qPrepZ * s010) + 2 * qPrepY * s020 +
					   qPrepW * (s000 - s100) + qPrepZ * s110 -
					   2 * qPrepY * s120 + 2 * q0w * s000 * theta -
					   2 * q0z * s010 * theta),
				q0w * qPrepY * s001 - q0x * qPrepZ * s001 - q0x * qPrepW * s011 -
				q0w * qPrepX * s011 + 2 * q0x * qPrepX * s021 -
				q0w * qPrepY * s101 + q0x * qPrepZ * s101 +
				q0x * qPrepW * s111 + q0w * qPrepX * s111 -
				2 * q0x * qPrepX * s121 - 2 * qPrepW * qPrepY * s001 * theta +
				2 * qPrepX * qPrepZ * s001 * theta -
				2 * q0w * q0x * s011 * theta +
				2 * qPrepW * qPrepX * s011 * theta +
				2 * qPrepY * qPrepZ * s011 * theta +
				2 * q0x * q0x * s021 * theta + 2 * q0y * q0y * s021 * theta -
				2 * qPrepX * qPrepX * s021 * theta -
				2 * qPrepY * qPrepY * s021 * theta +
				q0z * (-(qPrepX * s001) - qPrepY * s011 + qPrepX * s101 +
					   qPrepY * s111 - 2 * q0x * s001 * theta) +
				q0y * (-(qPrepZ * s011) + 2 * qPrepY * s021 +
					   qPrepW * (s001 - s101) + qPrepZ * s111 -
					   2 * qPrepY * s121 + 2 * q0w * s001 * theta -
					   2 * q0z * s011 * theta),
				q0w * qPrepY * s002 - q0x * qPrepZ * s002 - q0x * qPrepW * s012 -
				q0w * qPrepX * s012 + 2 * q0x * qPrepX * s022 -
				q0w * qPrepY * s102 + q0x * qPrepZ * s102 +
				q0x * qPrepW * s112 + q0w * qPrepX * s112 -
				2 * q0x * qPrepX * s122 - 2 * qPrepW * qPrepY * s002 * theta +
				2 * qPrepX * qPrepZ * s002 * theta -
				2 * q0w * q0x * s012 * theta +
				2 * qPrepW * qPrepX * s012 * theta +
				2 * qPrepY * qPrepZ * s012 * theta +
				2 * q0x * q0x * s022 * theta + 2 * q0y * q0y * s022 * theta -
				2 * qPrepX * qPrepX * s022 * theta -
				2 * qPrepY * qPrepY * s022 * theta +
				q0z * (-(qPrepX * s002) - qPrepY * s012 + qPrepX * s102 +
					   qPrepY * s112 - 2 * q0x * s002 * theta) +
				q0y * (-(qPrepZ * s012) + 2 * qPrepY * s022 +
					   qPrepW * (s002 - s102) + qPrepZ * s112 -
					   2 * qPrepY * s122 + 2 * q0w * s002 * theta -
					   2 * q0z * s012 * theta));

			c5[2] = DerivativeTerm(
				0., 2 * (qPrepW * qPrepY * s000 - qPrepX * qPrepZ * s000 +
						 q0y * q0z * s010 - qPrepW * qPrepX * s010 -
						 qPrepY * qPrepZ * s010 - q0y * q0y * s020 +
						 qPrepX * qPrepX * s020 + qPrepY * qPrepY * s020 +
						 q0x * q0z * (s000 - s100) - qPrepW * qPrepY * s100 +
						 qPrepX * qPrepZ * s100 +
						 q0w * (q0y * (-s000 + s100) + q0x * (s010 - s110)) -
						 q0y * q0z * s110 + qPrepW * qPrepX * s110 +
						 qPrepY * qPrepZ * s110 + q0y * q0y * s120 -
						 qPrepX * qPrepX * s120 - qPrepY * qPrepY * s120 +
						 q0x * q0x * (-s020 + s120)) *
					theta,
				2 * (qPrepW * qPrepY * s001 - qPrepX * qPrepZ * s001 +
					 q0y * q0z * s011 - qPrepW * qPrepX * s011 -
					 qPrepY * qPrepZ * s011 - q0y * q0y * s021 +
					 qPrepX * qPrepX * s021 + qPrepY * qPrepY * s021 +
					 q0x * q0z * (s001 - s101) - qPrepW * qPrepY * s101 +
					 qPrepX * qPrepZ * s101 +
					 q0w * (q0y * (-s001 + s101) + q0x * (s011 - s111)) -
					 q0y * q0z * s111 + qPrepW * qPrepX * s111 +
					 qPrepY * qPrepZ * s111 + q0y * q0y * s121 -
					 qPrepX * qPrepX * s121 - qPrepY * qPrepY * s121 +
					 q0x * q0x * (-s021 + s121)) *
				theta,
				2 * (qPrepW * qPrepY * s002 - qPrepX * qPrepZ * s002 +
					 q0y * q0z * s012 - qPrepW * qPrepX * s012 -
					 qPrepY * qPrepZ * s012 - q0y * q0y * s022 +
					 qPrepX * qPrepX * s022 + qPrepY * qPrepY * s022 +
					 q0x * q0z * (s002 - s102) - qPrepW * qPrepY * s102 +
					 qPrepX * qPrepZ * s102 +
					 q0w * (q0y * (-s002 + s102) + q0x * (s012 - s112)) -
					 q0y * q0z * s112 + qPrepW * qPrepX * s112 +
					 qPrepY * qPrepZ * s112 + q0y * q0y * s122 -
					 qPrepX * qPrepX * s122 - qPrepY * qPrepY * s122 +
					 q0x * q0x * (-s022 + s122)) *
				theta);
		}
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
				scale.m[i][j] = Lerp(dt, scaleStart.m[i][j], scaleEnd.m[i][j]);
		}
		*transform = Translate(translate) * rotate.ToTransform() * Transform(scale);
	}

	Bounds3f AnimatedTransform::MotionBounds(const Bounds3f& bound) const
	{
		if(!actuallyAnimated)
			return startTransform->operator()(bound);
		if(!hasRotation)
			return Union(startTransform->operator()(bound), endTransform->operator()(bound));
		Bounds3f bounds;
		for(uint32_t i = 0; i < 8; i++)
			bounds = Union(bounds, BoundPointMotion(bound.Corner(i)));
		return bounds;
	}

	Bounds3f AnimatedTransform::BoundPointMotion(const Point3f& point) const
	{
		Bounds3f bound(startTransform->operator()(point), endTransform->operator()(point));
		Float theta = std::acos(Clamp(Dot(rotateStart, rotateEnd), -1.f, 1.f));
		for(uint32_t i = 0; i < 3; i++)
		{
			//find any motion derivative zeros for the component
			Float zeros[4];
			uint32_t nZeros = 0;
			IntervalFindZeros(c1[i].Evaluate(point), c2[i].Evaluate(point), c3[i].Evaluate(point),
							  c4[i].Evaluate(point), c5[i].Evaluate(point), theta, Interval(0.f, 1.f),
							  zeros, &nZeros);
			//expand bounding box for any motion derivative zeros found
			for(uint32_t j = 0; j < nZeros; j++)
			{
				Point3f p = this->operator()(Lerp(zeros[j], startTime, endTime), point);
				bound.pMin[i] = std::min(p[i], bound.pMin[i]);
				bound.pMax[i] = std::max(p[i], bound.pMax[i]);
			}
		}
		return bound;
	}
}