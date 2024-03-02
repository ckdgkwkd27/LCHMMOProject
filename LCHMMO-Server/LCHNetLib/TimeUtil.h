#pragma once
#define ZERO_TIMESTAMP std::chrono::milliseconds(0)

using TimeStampType = std::chrono::milliseconds;

class TimeUtil
{
public:
	static char* GetCurrentTimeStr()
	{
		auto currentTime = std::chrono::system_clock::now();
		static char buffer[128];

		auto transformed = currentTime.time_since_epoch().count() / 1000000;

		auto millis = transformed % 1000;

		std::time_t tt;
		tm timeinfo = {};
		tt = system_clock::to_time_t(currentTime);
		localtime_s(&timeinfo, &tt);
		strftime(buffer, 128, "%F %H:%M:%S", &timeinfo);
		sprintf_s(buffer, "%s:%03d", buffer, (int)millis);
		return buffer;
	}

	static TimeStampType GetCurrTimeStamp()
	{
		TimeStampType timestamp = duration_cast<TimeStampType>(system_clock::now().time_since_epoch());
		return timestamp;
	}
};

