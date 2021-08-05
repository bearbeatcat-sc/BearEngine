#ifndef _TIMER_H_
#define _TIMER_H_

class Timer
{
public:
	Timer(float targetTime);
	~Timer();
	void Update();
	bool IsTime();
	void Reset();
	float GetTime();
	float GetRatio();
	void SetTime(float time);

private:
	float m_TargetTime;
	float m_Time;
};

#endif