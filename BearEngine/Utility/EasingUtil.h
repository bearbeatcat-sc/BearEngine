#pragma once

class EasingUtil
{
public:
	//static float ease_in(float t, float start, float end,float maxTime);
	//static float ease_out(float t, float start, float end,float maxTime);
	//static float ease_in_out(float t, float start, float end, float maxTime);
	//static float ease_out_quart(float t, float start, float end, float maxTime);

	static float ease_in_none(float x);
	static float ease_in_cubic(float x);
	static float ease_in_elastic(float x);
	static float ease_out_sin(float x);
	static float ease_out_elastic(float x);
	static float ease_inout_cublic(float x);
	static float ease_inout_elastic(float x);
	static float ease_out_quart(float x);

	static const float PI;

};