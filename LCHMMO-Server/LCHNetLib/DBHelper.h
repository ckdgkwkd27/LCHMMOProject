#pragma once
#include <sqlext.h>

struct SqlConn
{
	SQLHDBC sqlHdbc = SQL_NULL_HANDLE;
	SQLHSTMT sqlHstmt = SQL_NULL_HANDLE;
};
class DBHelper
{
public:
	DBHelper();
	~DBHelper();

	static bool Initialize(const wchar_t* connInfo, uint32 dbThreadCnt);
	bool Execute(Wstring sqlStmt);
	bool FetchRow();

	bool BindParamInt(int* param);
	bool BindParamFloat(float* param);
	bool BindParamBool(bool* param);
	bool BindParamTextA(const char* text);
	bool BindParamTextW(const wchar_t* text);

	void BindResultColumnInt(int* r);
	void BindResultColumnFloat(float* r);
	void BindResultColumnBool(bool* r);
	void BindResultColumnText(wchar_t* text, size_t count);

	void PrintSqlStmtError();

private:
	SQLHSTMT currSqlHstmt;
	SQLSMALLINT currResultCol;
	SQLSMALLINT currBindParam;

	static SQLHENV sqlHenv;
	static SqlConn* sqlConnPool;
	static uint32 DBWorkerThreadCnt;
};