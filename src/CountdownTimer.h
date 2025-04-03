#pragma once

class CountdownTimer
{
public:
	CountdownTimer(const float durationSeconds, const bool initStarted)
	{
		m_duration = durationSeconds;
		m_remaining = initStarted ? durationSeconds : 0;
	}

	void Update(const float deltaTime)
	{
		m_remaining -= deltaTime;
	}

	void ForceEnd()
	{
		m_remaining = 0;
	}

	void Reset()
	{
		m_remaining = m_duration;
	}

	bool IsFinished() const
	{
		return m_remaining <= 0;
	}

	float GetRemaining() const
	{
		return m_remaining;
	}

	float GetProgress() const
	{
		return 1.0f - (m_remaining / m_duration);
	}

	float GetInvProgress() const
	{
		return m_remaining / m_duration;
	}

	void SetNewDuration(const float durationSeconds)
	{
		m_duration = durationSeconds;
	}

private:
	float m_duration;
	float m_remaining;
};
