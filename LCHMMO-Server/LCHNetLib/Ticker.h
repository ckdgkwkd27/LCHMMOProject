#pragma once

class Ticker
{
public:
	using TickIntervalType = std::chrono::duration<int64, std::nano>;
	using OnTickFunction = std::function<void()>;

	Ticker(OnTickFunction _onTickFunction, TickIntervalType _tickInterval);
	void Start();
	void Stop();
	void SetDuration(TickIntervalType _tickInterval);

private:
	void Loop();

	TickIntervalType tickInterval;
	OnTickFunction onTickFunction;
	bool isRunning;
	RecursiveMutex tickLock;
};

