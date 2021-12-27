﻿#ifndef _TIME_H_
#define _TIME_H_

#include <d3dx12.h>


class Time
{
public:
	Time();
	~Time();
	static void Update();
	static float GetNow();

public:
	static float DeltaTime;
	static float TimeScale;
	static int FPS;

private:
	static float lastTime;
	static float BackTickCount;
	static int FPSCounter;

	static LARGE_INTEGER TimeFreq;
	static LARGE_INTEGER TimeStart;

	// 近似化のための係数
	static const float k ;
	static float avgTime;
};

#endif