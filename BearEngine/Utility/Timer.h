#ifndef _TIMER_H_
#define _TIMER_H_

class Timer
{
public:
	Timer(float targetTime);
	~Timer() = default;

	void Update();
	bool IsTime() const;
	void Reset();
	float GetTime() const;
	float GetRatio() const;
	void SetTime(float time);

private:
	float m_TargetTime;
	float m_Time;
};

#endif