#ifndef _MATHUTILITY_H_
#define _MATHUTILITY_H_

#include <SimpleMath.h>
#include <vector>

using namespace DirectX;


class MathUtility
{
public:
	static const float PI;
	static const float TwoPI;
	static const float PiOver2;
	static const float Infinity;
	static const float NegInfinity;

	static const float ToRadians(float degree);
	static const float ToDegress(float radians);
	static const SimpleMath::Vector3& GetMin(const std::vector<SimpleMath::Vector3>& vectors);
	static const SimpleMath::Vector3& GetMin(const std::vector<XMFLOAT3>& vectors);
	static const SimpleMath::Vector3& GetMax(const std::vector<SimpleMath::Vector3>& vectors);
	static const SimpleMath::Vector3& GetMax(const std::vector<XMFLOAT3>& vectors);

	// 近似値かどうか
	static const bool NaerZero(float val, float epsilon = 0.001f);

	template<typename T>
	static const T Max(const T& a, const T& b);
	template<typename T>
	static const T Min(const T& a, const T& b);
	template<typename T>
	static const T Clamp(const T& value, const T& lower, const T& upper);

	static float GetRadian(float x, float y, float x2, float y2);
	static float GetAngle(float x, float y, float x2, float y2);
	static float Abs(float value);
	static float Cos(float angle);
	static float Acos(float value);
	static float Sin(float angle);
	static float Atan2(float x, float y);
	static float Tan(float angle);
	static float Cot(float angle);
	static float Lerp(float a, float b, float f);
	static float Sqrt(float value);
	static float Fmod(float numer, float denom);
	static DirectX::SimpleMath::Vector3 ScreenToWorld(int screenX, int screenY, float fZ, int screenW, int screenH, float _far, float _near, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);
	static DirectX::SimpleMath::Quaternion LookAt(const DirectX::SimpleMath::Vector3& sourePoint, const DirectX::SimpleMath::Vector3& targetPoint,const DirectX::SimpleMath::Vector3& up = SimpleMath::Vector3::Up);
	static  std::vector<float> GetGaussianWeights(size_t count, float s);

};

#endif

template<typename T>
inline const T MathUtility::Max(const T& a, const T& b)
{
	return (a < b ? b : a);
}

template<typename T>
inline const T MathUtility::Min(const T& a, const T& b)
{
	return (a < b ? a : b);
}

template<typename T>
inline const T MathUtility::Clamp(const T& value, const T& lower, const T& upper)
{
	return Min(upper, Max(lower, value));
}

