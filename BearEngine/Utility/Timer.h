#pragma once

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

