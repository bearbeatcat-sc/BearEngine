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

Time::Time()
{
	Time::lastTime = timeGetTime();
}

Time::~Time()
{
}

void Time::Update()
{

	float currTime = timeGetTime();
	float deltaTime = (currTime - Time::lastTime) / 1000.0f;
	Time::lastTime = currTime;

	FPSCounter++;
	if (currTime - BackTickCount >= 1000)
	{
		BackTickCount = currTime;
		FPS = FPSCounter;
		FPSCounter = 0;
	}

	if (deltaTime < LOW_LIMIT) deltaTime = LOW_LIMIT;
	if (deltaTime > HIGH_LIMIT) deltaTime = HIGH_LIMIT;

	Time::DeltaTime = deltaTime * Time::TimeScale;

}

float Time::GetNow()
{
	return timeGetTime();
}
