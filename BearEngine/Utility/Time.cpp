#include "Time.h"
#include <Windows.h>


float Time::DeltaTime = 0.0f;
float Time::TimeScale = 1.0f;
float Time::ElapsedTime = 0.0f;

double Time::lastTime = 0.0f;
int Time::FPS = 0;

const float Time::k = 0.1f;
float Time::avgTime = 0.0f;

float LOW_LIMIT = 0.0167f;
float HIGH_LIMIT = 0.1f;

LARGE_INTEGER Time::TimeFreq;
LARGE_INTEGER Time::TimeStart;

Time::Time()
{
	QueryPerformanceFrequency(&TimeFreq);
	QueryPerformanceCounter(&TimeStart);
}

Time::~Time()
{
}

void Time::Update()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	//float deltaTime = (currentTime - Time::lastTime) / 1000.0f;
	const double elasped = static_cast<double>(currentTime.QuadPart - TimeStart.QuadPart) / static_cast<double>(TimeFreq.QuadPart);

	const float deltaTime = static_cast<float>(elasped - lastTime);
	lastTime = elasped;

	avgTime *= 1.0f - k;
	avgTime += deltaTime * k;

	FPS = 1.0f / avgTime;


	//FPSCounter++;
	//if (currTime - BackTickCount >= 1000)
	//{
	//	BackTickCount = currTime;
	//	FPS = FPSCounter;
	//	FPSCounter = 0;
	//}

	//if (deltaTime < LOW_LIMIT) deltaTime = LOW_LIMIT;
	//if (deltaTime > HIGH_LIMIT) deltaTime = HIGH_LIMIT;

	Time::DeltaTime = deltaTime;

	ElapsedTime += Time::DeltaTime;
}

float Time::GetNow()
{
	return timeGetTime();
}
