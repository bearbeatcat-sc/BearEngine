#include "MathUtility.h"

#include <cmath>
#include <limits>
#include <vector>


const float MathUtility::PI = 3.1415926535f;
const float MathUtility::TwoPI = PI * 2.0f;
const float MathUtility::PiOver2 = PI / 2.0f;
const float MathUtility::Infinity = std::numeric_limits<float>::infinity();
const float MathUtility::NegInfinity = -std::numeric_limits<float>::infinity();

using namespace DirectX::SimpleMath;

const float MathUtility::ToRadians(float degree)
{
	return degree * PI / 180.0f;
}

const float MathUtility::ToDegress(float radians)
{
	return radians * 180.0f / PI;
}

const SimpleMath::Vector3& MathUtility::GetMin(const std::vector<SimpleMath::Vector3>& vectors)
{
	// float型の最大値
	SimpleMath::Vector3 temp;
	temp.x = FLT_MAX;
	temp.y = FLT_MAX;
	temp.z = FLT_MAX;

	for (auto vec : vectors)
	{
		if (vec.x < temp.x)
		{
			temp.x = vec.x;
		}

		if (vec.y < temp.y)
		{
			temp.y = vec.y;
		}

		if (vec.z < temp.z)
		{
			temp.z = vec.z;
		}
	}

	return temp;
}

const SimpleMath::Vector3& MathUtility::GetMin(const std::vector<XMFLOAT3>& vectors)
{
	// float型の最大値
	SimpleMath::Vector3 temp;
	temp.x = FLT_MAX;
	temp.y = FLT_MAX;
	temp.z = FLT_MAX;

	for (auto vec : vectors)
	{
		if (vec.x < temp.x)
		{
			temp.x = vec.x;
		}

		if (vec.y < temp.y)
		{
			temp.y = vec.y;
		}

		if (vec.z < temp.z)
		{
			temp.z = vec.z;
		}
	}

	return temp;
}

const SimpleMath::Vector3& MathUtility::GetMax(const std::vector<SimpleMath::Vector3>& vectors)
{	
	SimpleMath::Vector3 temp;


	for (auto vec : vectors)
	{
		if (vec.x > temp.x)
		{
			temp.x = vec.x;
		}

		if (vec.y > temp.y)
		{
			temp.y = vec.y;
		}

		if (vec.z > temp.z)
		{
			temp.z = vec.z;
		}
	}

	return temp;
}

const SimpleMath::Vector3& MathUtility::GetMax(const std::vector<XMFLOAT3>& vectors)
{
	SimpleMath::Vector3 temp;


	for (auto vec : vectors)
	{
		if (vec.x > temp.x)
		{
			temp.x = vec.x;
		}

		if (vec.y > temp.y)
		{
			temp.y = vec.y;
		}

		if (vec.z > temp.z)
		{
			temp.z = vec.z;
		}
	}

	return temp;
}


const bool MathUtility::NaerZero(float val, float epsilon)
{
	if (fabs(val) <= epsilon)
	{
		return true;
	}

	return false;
}

float MathUtility::GetRadian(float x, float y, float x2, float y2)
{
	float distance = MathUtility::Sqrt((x2 - x) * (x2 - x) + (y2 - y) * (y2 - y));

	return distance;
}

float MathUtility::GetAngle(float x, float y, float x2, float y2)
{
	float radius = atan2f(y2 - y, x2 - x);

	if (radius < 0)
	{
		radius = radius + 2 * PI;
	}

	return floor(radius * 360.0f / (2.0f * PI));
}

float MathUtility::Abs(float value)
{
	return fabs(value);
}

float MathUtility::Cos(float angle)
{
	return cosf(angle);
}

float MathUtility::Acos(float value)
{
	return acosf(value);
}

float MathUtility::Sin(float angle)
{
	return sinf(angle);
}

float MathUtility::Atan2(float x, float y)
{
	return atan2f(y,x);
}

float MathUtility::Tan(float angle)
{
	return tanf(angle);
}

float MathUtility::Cot(float angle)
{
	return 1.0f / Tan(angle);
}

float MathUtility::Lerp(float a, float b, float f)
{
	return a + f * (b-a);
}

float MathUtility::Sqrt(float value)
{
	return sqrtf(value);
}

float MathUtility::Fmod(float numer, float denom)
{
	return fmod(numer,denom);
}

DirectX::SimpleMath::Vector3 MathUtility::ScreenToWorld(int screenX, int screenY, float fZ, int screenW, int screenH, float _far, float _near, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
{
	// 逆行列
	//DirectX::SimpleMath::Matrix invView, invPrj, vp, InvViewPort;

	//view.Invert(invView);
	//proj.Invert(invPrj);
	//vp = DirectX::SimpleMath::Matrix::Identity;

	//vp.m[0][0] = vp.m[0][3] = screenW / 2.0f;
	//vp.m[1][1] = screenH / 2.0f;
	//vp.m[1][3] = screenH / 2.0f;

	//vp.m[2][2] = (_far - _near) / 2.0f;
	//vp.m[2][3] = (_far + _near) / 2.0f;
	//vp.Invert(InvViewPort);

	//// 逆変換
	//auto temp = InvViewPort * invPrj * invView;

	fZ = _near;
	DirectX::SimpleMath::Vector3 pos = DirectX::SimpleMath::Vector3(screenX, screenY, fZ);

	//float x = pos.x * temp.m[0][0] + pos.y * temp.m[0][1] + pos.z * temp.m[0][2] + temp.m[0][3];
	//float y = pos.x * temp.m[1][0] + pos.y * temp.m[1][1] + pos.z * temp.m[1][2] + temp.m[1][3];
	//float z = pos.x * temp.m[2][0] + pos.y * temp.m[2][1] + pos.z * temp.m[2][2] + temp.m[2][3];
	//float w = pos.x * temp.m[3][0] + pos.y * temp.m[3][1] + pos.z * temp.m[3][2] + temp.m[3][3];

	//x /= w;
	//y /= w;
	//z /= w;

	return DirectX::XMVector3Unproject(pos, 0, 0, screenW, screenH, _near, _far, proj, view, Matrix::Identity);
	//return	DirectX::SimpleMath::Vector3::Transform(pos, temp);
}

DirectX::SimpleMath::Quaternion MathUtility::LookAt(DirectX::SimpleMath::Vector3 sourePoint, DirectX::SimpleMath::Vector3 targetPoint)
{
	//// 進行方向ベクトル
	//DirectX::SimpleMath::Vector3 forwardVector = (sourePoint - targetPoint);
	//forwardVector.Normalize();

	//// Forwardベクトル（0,0,1)との内積
	//float dot = DirectX::SimpleMath::Vector3(0, 0, 1).Dot(forwardVector);

	//// 平行かによって、返す値が違う？
	//if (MathUtility::Abs(dot - (-1.0f)) < 0.000001f)
	//{
	//	return DirectX::SimpleMath::Quaternion(DirectX::SimpleMath::Vector3(0, 1, 0), 3.1415926535897932f);
	//}

	//if (MathUtility::Abs(dot + (1.0f)) < 0.000001f)
	//{
	//	return DirectX::SimpleMath::Quaternion::Identity;
	//}

	//float rotAngle = (float)MathUtility::Acos(dot);
	//DirectX::SimpleMath::Vector3 rotAxis = DirectX::SimpleMath::Vector3(0, 0, 1).Cross(forwardVector);
	//rotAxis.Normalize();

	//return DirectX::SimpleMath::Quaternion(rotAxis, rotAngle);

	DirectX::SimpleMath::Vector3 z = (targetPoint - sourePoint);
	z.Normalize();

	DirectX::SimpleMath::Vector3 x = z.Cross(Vector3::Up);
	x.Normalize();

	DirectX::SimpleMath::Vector3 y = z.Cross(x);
	y.Normalize();

	Matrix m = Matrix::Identity;
	m.m[0][0] = x.x; m.m[0][1] = y.x; m.m[0][2] = z.x;
	m.m[1][0] = x.y; m.m[1][1] = y.y; m.m[1][2] = z.y;
	m.m[2][0] = x.z; m.m[2][1] = y.z; m.m[2][2] = z.z;

	return Quaternion::CreateFromRotationMatrix(m);
}

 std::vector<float> MathUtility::GetGaussianWeights(size_t count, float s)
{
	std::vector<float> weights(count);
	float x = 0.0f;
	float total = 0.0f;

	for (auto& wgt : weights)
	{
		wgt = expf(-(x * x) / (2 * s * s));
		total += wgt;
		x += 1.0f;
	}

	total = total * 2.0f - 1;

	for (auto& wgt : weights)
	{
		wgt /= total;
	}

	return weights;
}
