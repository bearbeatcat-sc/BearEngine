#include "Timer.h"
#include "Time.h"

Timer::Timer(float targetTime)
	:m_TargetTime(targetTime),m_Time(0.0f)
{
}


void Timer::Update()
{
	m_Time += Time::DeltaTime;
}

bool Timer::IsTime() const 
{
	return m_Time >= m_TargetTime;
}

void Timer::SetTime(float time)
{
	m_TargetTime = time;
}

void Timer::Reset()
{
	m_Time = 0.0f;
}

float Timer::GetTime() const
{
	return m_Time;
}

float Timer::GetRatio() const
{
	return m_Time / m_TargetTime;
}
