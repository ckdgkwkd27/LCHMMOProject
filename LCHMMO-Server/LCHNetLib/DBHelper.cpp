#include "pch.h"
#include "DBHelper.h"

SQLHENV DBHelper::sqlHenv;
SqlConn* DBHelper::sqlConnPool;
uint32 DBHelper::DBWorkerThreadCnt = 0;

DBHelper::DBHelper()
{
	currResultCol = 1;
	currBindParam = 1;

	currSqlHstmt = sqlConnPool[LWorkerThreadId].sqlHstmt;
}

DBHelper::~DBHelper()
{
	SQLFreeStmt(currSqlHstmt, SQL_UNBIND);
	SQLFreeStmt(currSqlHstmt, SQL_RESET_PARAMS);
	SQLFreeStmt(currSqlHstmt, SQL_CLOSE);
}

bool DBHelper::Initialize(const wchar_t* connInfo, uint32 dbThreadCnt)
{
	sqlConnPool = new SqlConn[dbThreadCnt];

	if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlHenv))
	{
		printf_s("DbHelper Initialize SQLAllocHandle failed\n");
		return false;
	}

	if (SQL_SUCCESS != SQLSetEnvAttr(sqlHenv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
	{
		printf_s("DbHelper Initialize SQLSetEnvAttr failed\n");
		return false;
	}

	/// 스레드별로 SQL connection을 풀링하는 방식
	for (uint32 i = 0; i < dbThreadCnt; ++i)
	{
		if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlHenv, &sqlConnPool[i].sqlHdbc))
		{
			printf_s("DbHelper Initialize SQLAllocHandle failed\n");
			return false;
		}

		SQLSMALLINT resultLen = 0;
		SQLRETURN ret = SQLDriverConnect(sqlConnPool[i].sqlHdbc, NULL, (SQLWCHAR*)connInfo, (SQLSMALLINT)wcslen(connInfo), NULL, 0, &resultLen, SQL_DRIVER_NOPROMPT);
		if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
		{
			SQLWCHAR sqlState[1024] = { 0, };
			SQLINTEGER nativeError = 0;
			SQLWCHAR msgText[1024] = { 0, };
			SQLSMALLINT textLen = 0;

			SQLGetDiagRec(SQL_HANDLE_DBC, sqlConnPool[i].sqlHdbc, 1, sqlState, &nativeError, msgText, 1024, &textLen);
			wprintf_s(L"DbHelper Initialize SQLDriverConnect failed: %s \n", msgText);
			return false;
		}

		if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnPool[i].sqlHdbc, &sqlConnPool[i].sqlHstmt))
		{
			printf_s("DbHelper Initialize SQLAllocHandle SQL_HANDLE_STMT failed\n");
			return false;
		}
	}

	return true;
}

bool DBHelper::Execute(Wstring sqlStmt)
{
	SQLRETURN ret = SQLExecDirectW(currSqlHstmt, (SQLWCHAR*)sqlStmt.c_str(), SQL_NTSL);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DBHelper::FetchRow()
{
	SQLRETURN ret = SQLFetch(currSqlHstmt);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		if (SQL_NO_DATA != ret)
		{
			PrintSqlStmtError();
		}

		return false;
	}

	return true;
}

bool DBHelper::BindParamInt(int* param)
{
	SQLRETURN ret = SQLBindParameter(currSqlHstmt, currBindParam++, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 10, 0, param, 0, NULL);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DBHelper::BindParamFloat(float* param)
{
	SQLRETURN ret = SQLBindParameter(currSqlHstmt, currBindParam++, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 15, 0, param, 0, NULL);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DBHelper::BindParamBool(bool* param)
{
	SQLRETURN ret = SQLBindParameter(currSqlHstmt, currBindParam++, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 3, 0, param, 0, NULL);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DBHelper::BindParamTextA(const char* text)
{
	SQLRETURN ret = SQLBindParameter(sqlConnPool[0].sqlHstmt, currBindParam++, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, strlen(text), 0, (SQLPOINTER)text, 0, NULL);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

bool DBHelper::BindParamTextW(const wchar_t* text)
{
	SQLRETURN ret = SQLBindParameter(currSqlHstmt, currBindParam++, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, wcslen(text), 0, (SQLPOINTER)text, 0, NULL);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
		return false;
	}

	return true;
}

void DBHelper::BindResultColumnInt(int* r)
{
	SQLLEN len = 0;
	SQLRETURN ret = SQLBindCol(currSqlHstmt, currResultCol++, SQL_C_LONG, r, 4, &len);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}

void DBHelper::BindResultColumnFloat(float* r)
{
	SQLLEN len = 0;
	SQLRETURN ret = SQLBindCol(currSqlHstmt, currResultCol++, SQL_C_FLOAT, r, 4, &len);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}

void DBHelper::BindResultColumnBool(bool* r)
{
	SQLLEN len = 0;
	SQLRETURN ret = SQLBindCol(currSqlHstmt, currResultCol++, SQL_C_TINYINT, r, 1, &len);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}

void DBHelper::BindResultColumnText(wchar_t* text, size_t count)
{
	SQLLEN len = 0;
	SQLRETURN ret = SQLBindCol(currSqlHstmt, currResultCol++, SQL_C_WCHAR, text, count * 2, &len);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		PrintSqlStmtError();
	}
}

void DBHelper::PrintSqlStmtError()
{
	SQLWCHAR sqlState[1024] = { 0, };
	SQLINTEGER nativeError = 0;
	SQLWCHAR msgText[1024] = { 0, };
	SQLSMALLINT textLen = 0;

	SQLGetDiagRecW(SQL_HANDLE_STMT, currSqlHstmt, 1, sqlState, &nativeError, msgText, _countof(msgText), &textLen);

	std::wcout.imbue(std::locale("kor"));
	std::wcout << L"DB ERROR: " << msgText << std::endl;
}
