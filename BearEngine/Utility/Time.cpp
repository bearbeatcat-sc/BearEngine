#include "Time.h"
#include "../imgui/imgui.h"
#include <Windows.h>

float Time::DeltaTime = 0.0f;
float Time::TimeScale = 1.0f;
float Time::lastTime = 0.0f;
float Time::BackTickCount = 0.0f;
int Time::FPS = 0.0f;
int Time::FPSCounter = 0.0f;
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
	float deltaTime = static_cast<double>(currentTime.QuadPart - TimeStart.QuadPart) / static_cast<double>(TimeFreq.QuadPart);
	Time::TimeStart = currentTime;

	FPS = 1 / deltaTime;

	//FPSCounter++;
	//if (currTime - BackTickCount >= 1000)
	//{
	//	BackTickCount = currTime;
	//	FPS = FPSCounter;
	//	FPSCounter = 0;
	//}

	//if (deltaTime < LOW_LIMIT) deltaTime = LOW_LIMIT;
	//if (deltaTime > HIGH_LIMIT) deltaTime = HIGH_LIMIT;

	Time::DeltaTime = deltaTime * Time::TimeScale;

}

float Time::GetNow()
{
	return timeGetTime();
}
