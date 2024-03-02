#pragma once
#include "DumpUtil.h"

inline static bool CRASH_ASSERT(bool expr)
{
	if (!expr)
	{
		int* crashVal = nullptr;
		*crashVal = 0xDEADBEEF;
	}
	return true;
}

extern thread_local uint32 LWorkerThreadId;