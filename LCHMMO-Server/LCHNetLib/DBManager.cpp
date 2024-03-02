#include "pch.h"
#include "DBManager.h"
#include "IocpManager.h"
#include "DBHelper.h"

DBManager* GDBManager = nullptr;

DBManager::DBManager() 
	: dbHandle(INVALID_HANDLE_VALUE)
{
	ZeroMemory(DBWorkerThreads, sizeof(DBWorkerThreads));
}

DBManager::~DBManager()
{
	for (uint32 idx = 0; idx < MAX_DB_THREADS; idx++)
	{
		this->DBWorkerThreads[idx].join();
	}
}

void DBManager::Initialize()
{
	dbHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CRASH_ASSERT(dbHandle != INVALID_HANDLE_VALUE);

	CRASH_ASSERT(DBHelper::Initialize(SQL_SERVER_CONN_STR, MAX_DB_THREADS));

	std::cout << "[DB] DBManager Initialized!" << std::endl;
}

bool DBManager::Start()
{
	for (uint32 idx = 0; idx < MAX_DB_THREADS; idx++)
	{
		DBWorkerThreads[idx] = std::thread(&DBManager::DBWorkerThread, this, idx);
	}

	std::cout << "[DB] DBWorkerThread Runs!" << std::endl;
	return true;
}

void DBManager::PostDBMessage(DBMessage* message)
{
	if (FALSE == PostQueuedCompletionStatus(dbHandle, sizeof(message), (ULONG_PTR)CompletionKeyType::CK_DB, (LPOVERLAPPED)message))
	{
		printf("DBManager::PostDatabsaseRequest PostQueuedCompletionStatus Error: %d\n", GetLastError());
		CRASH_ASSERT(false);
	}
}

void DBManager::DBWorkerThread(uint32 id)
{
	while (true)
	{
		DWORD dwTransferred = 0;
		LPOVERLAPPED overlapped = nullptr;
		ULONG_PTR completionKey = 0;
		LWorkerThreadId = id;

		bool ret = GetQueuedCompletionStatus(dbHandle, &dwTransferred, (PULONG_PTR)&completionKey, &overlapped, INFINITE);
		if (ret == false)
		{
			int32 errCode = GetLastError();
			switch (errCode)
			{
			case WAIT_TIMEOUT:
				std::cout << "[FAIL] GQCS Timeout Fail" << std::endl;
				return;
			case ERROR_NETNAME_DELETED:
				break;
			default:
				std::cout << "[FAIL] GQCS ErrorCode: " << errCode << std::endl;
				break;
			}
		}

		DBMessage* dbMessage = reinterpret_cast<DBMessage*>(overlapped);
		dbMessage->isSuccess = dbMessage->SqlExecute();
		GIocpManager->PostDBResult(dbMessage);
	}
}
