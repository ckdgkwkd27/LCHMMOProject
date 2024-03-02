#pragma once
#include "DBMessage.h"
#define MAX_DB_THREADS 1
#define SQL_SERVER_CONN_STR	L"DRIVER={SQL Server};SERVER=ckdgkwkd27;DATABASE=master;Trusted_Connection=yes;"

class DBManager
{
public:
	DBManager();
	~DBManager();

	void Initialize();
	bool Start();
	void PostDBMessage(DBMessage* message);
	void DBWorkerThread(uint32 id);

private:
	HANDLE dbHandle;
	std::thread DBWorkerThreads[MAX_DB_THREADS];

};

extern DBManager* GDBManager;
