#include "pch.h"
#include "DumpUtil.h"

DumpUtil GDumpUtil;

DumpUtil::DumpUtil()
{
	::SetUnhandledExceptionFilter(execptionFilter);
	printf("* Dump filter setting complte!\n");
}

LONG WINAPI DumpUtil::execptionFilter(struct _EXCEPTION_POINTERS* exceptionInfo)
{
	_CrtMemDumpAllObjectsSince(NULL);

	HMODULE dumpDll = nullptr;
	dumpDll = ::LoadLibraryA("DBGHELP.DLL");
	if (!dumpDll) {
		printf("! DBGHelp.dll not loaded\n");
		return 0;
	}

	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	struct tm tmCurrTime;
	localtime_s(&tmCurrTime, &in_time_t);

	std::wstringstream lss;
	lss << std::put_time(&tmCurrTime, L"%Y-%m-%d %H.%M.%S"); //%Y-%m-%d_%X

	Wstring dumpPatch;
#ifdef _DEBUG
	dumpPatch = L"../Output/bin/debug/";
#endif
#ifndef _DEBUG
	dumpPatch = L"../Output/bin/release/";
#endif
	dumpPatch += lss.str();
	dumpPatch += L".dmp";
	std::wcout << "Dump FileName: " << dumpPatch.c_str() << std::endl;

	HANDLE file = ::CreateFile2(dumpPatch.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		printf("! dump file not making: %d\n", GetLastError());
		return 0;
	}

	_MINIDUMP_EXCEPTION_INFORMATION info;
	info.ThreadId = ::GetCurrentThreadId();
	info.ExceptionPointers = exceptionInfo;
	info.ClientPointers = NULL;

	WRITEDUMP dumpFunc = (WRITEDUMP)::GetProcAddress(dumpDll, "MiniDumpWriteDump");
	if (dumpFunc(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, &info, NULL, NULL) == FALSE) {
		printf("! dump file saving error\n");
		return 0;
	}
	::CloseHandle(file);

	return EXCEPTION_CONTINUE_SEARCH;
}
