#pragma once
#include <DbgHelp.h>
#include <sstream>
#include <iomanip>
#pragma comment(lib, "Dbghelp.lib")

// 참고 http://msdn.microsoft.com/ko-kr/library/windows/desktop/ms680360(v=vs.85).aspx

//dump 함수 포인터
typedef BOOL(WINAPI* WRITEDUMP)(
	_In_  HANDLE hProcess,
	_In_  DWORD ProcessId,
	_In_  HANDLE hFile,
	_In_  MINIDUMP_TYPE DumpType,
	_In_  PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	_In_  PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	_In_  PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);

class DumpUtil
{
public:
	DumpUtil();

	static LONG WINAPI execptionFilter(struct _EXCEPTION_POINTERS* exceptionInfo);
};

extern DumpUtil GDumpUtil;