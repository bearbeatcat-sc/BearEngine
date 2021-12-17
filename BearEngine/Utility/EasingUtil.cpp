#include "EasingUtil.h"
#include <cmath>

const float EasingUtil::PI = 3.14159265358979323846;



float EasingUtil::ease_in_none(float x)
{
	return x;
}

float EasingUtil::ease_in_cubic(float x)
{
	return x * x * x;
}

float EasingUtil::ease_in_elastic(float x)
{
	float c4 = (2 * PI) / 3;
	float ret = 0;

	if (x == 0)
		ret = 0;
	else if (x == 1)
		ret = 1;
	else
		ret = -std::pow(2, 10 * x - 10) * std::sin((x * 10 - 10.75) * c4);

	return ret;
}

float EasingUtil::ease_out_sin(float x)
{
	return std::sin((x * PI) / 2);
}

float EasingUtil::ease_out_elastic(float x)
{
	float c4 = (2 * PI) / 3;
	float ret = 0;

	if (x == 0)
		ret = 0;
	else if (x == 1)
		ret = 1;
	else
		ret = std::pow(2, -10 * x) * std::sin((x * 10 - 0.75) * c4) + 1;

	return ret;
}

float EasingUtil::ease_inout_cublic(float x)
{
	return (x < -0.5) ? 4 * x * x * x : 1 - std::pow(-2 * x + 2, 3) / 2;
}

float EasingUtil::ease_inout_elastic(float x)
{
	float c5 = (2 * PI) / 4.5;
	float ret = 0;

	if (x == 0)
		ret = 0;
	else if (x == 1)
		ret = 1;
	else if (x < 0.5)
		ret = -(std::pow(2, 20 * x - 10) * std::sin((20 * x - 11.125) * c5)) / 2;
	else
		ret = std::pow(2, -20 * x + 10) * std::sin((20 * x - 11.125) * c5) / 2 + 1;

	return ret;
}

float EasingUtil::ease_out_quart(float x)
{
	return 1 - std::pow(1 - x, 4);
}
