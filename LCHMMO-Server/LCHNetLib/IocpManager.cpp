#include "pch.h"
#include "IocpManager.h"
#include "TimeUtil.h"

char AcceptBuf[64] = { 0, };
IocpManager* GIocpManager;

IocpManager::IocpManager(std::wstring _ip, uint16 _port, uint32 _maxConnectionCnt)
{
	ip = _ip;
	port = _port;
	maxConnectionCnt = _maxConnectionCnt;
}

void IocpManager::Initialize()
{
	workerThreadsCnt = std::thread::hardware_concurrency();
	workerThreads.resize(workerThreadsCnt);

	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CRASH_ASSERT(iocpHandle != NULL);

	listenSocket = SocketUtil::CreateListenSocket();
	HANDLE _handle = CreateIoCompletionPort((HANDLE)listenSocket, iocpHandle, 0, 0);
	if (_handle != iocpHandle)
	{
		CRASH_ASSERT(false);
	}

	std::cout << "[INFO] IOCP Initialization(" << workerThreadsCnt << " Threads) Success.." << std::endl;
	return;
}

void IocpManager::BindAndListen()
{
	SocketUtil::Bind(listenSocket, ip, port);
	CRASH_ASSERT(SocketUtil::Listen(listenSocket));
	std::wcout << L"[INFO] Server Listen Start IP=" << ip << L", port=" << port << std::endl;
}

bool IocpManager::StartWorker()
{
	for (uint32 i = 0; i < workerThreadsCnt; i++)
	{
		workerThreads[i] = std::thread(&IocpManager::WorkerThreadFunc, this);
	}

	return true;
}

bool IocpManager::Join()
{
	for (uint32 i = 0; i < workerThreadsCnt; i++)
	{
		if (workerThreads[i].joinable())
		{
			workerThreads[i].join();
		}
	}

	return true;
}

void IocpManager::AcceptThreadFunc()
{
	while (GSessionManager.AcceptClientSession(maxConnectionCnt))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void IocpManager::Dispatch(IocpEvent* iocpEvent, DWORD bytes)
{
	SessionPtr _session = iocpEvent->sessionRef;
	if (_session == nullptr)
		return;

	switch (iocpEvent->GetType())
	{
	case EventType::ACCEPT:
	{
		_session->ProcessAccept();
		break;
	}
	case EventType::CONNECT:
	{
		_session->ProcessConnect();
		break;
	}
	case EventType::DISCONNECT:
	{
		_session->ProcessDisconnect();
		break;
	}
	case EventType::SEND:
	{
		_session->ProcessSend(bytes);
		sendTps.fetch_add(1);
		break;
	}
	case EventType::RECV:
	{
		_session->ProcessRecv(bytes);
		recvTps.fetch_add(1);
		break;
	}
	default:
		std::cout << "[FAIL] Wrong EventType: " << (uint8)iocpEvent->GetType() << std::endl;
		break;
	}
}

void IocpManager::WorkerThreadFunc()
{
	while (true)
	{
		DWORD bytes;
		ULONG_PTR key;
		IocpEvent* iocpEvent = nullptr;
		LPOVERLAPPED overlapped;

		bool ret = GetQueuedCompletionStatus(iocpHandle, &bytes, &key, &overlapped, INFINITE);
		iocpEvent = reinterpret_cast<IocpEvent*>(overlapped);
		CRASH_ASSERT(iocpEvent != NULL);
		CRASH_ASSERT(iocpEvent != INVALID_HANDLE_VALUE);

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

		if (key == (ULONG_PTR)CompletionKeyType::CK_LOOPBACK)
		{
			CircularBufferPtr buffer = iocpEvent->sessionRef->sessionSendEvent.sendBuffer;
			SessionPtr _session = iocpEvent->sessionRef;
			if (_session == nullptr)
				return;

			if (_session->ProcessLoopback(buffer, bytes) == false)
			{
				std::cout << "[FAIL] Loopback Job Failed!" << std::endl;
				return;
			}
			_session->PostRecv();
		}

		else if (key == (ULONG_PTR)CompletionKeyType::CK_DB)
		{
			DBMessage* dbContext = reinterpret_cast<DBMessage*>(overlapped);
			dbContext->OnResult();
		}

		else
		{ 
			Dispatch(iocpEvent, bytes);
		}
	}
}

bool IocpManager::IocpPost(CircularBufferPtr buffer, IocpEvent* iocpEvent)
{
	bool ret = PostQueuedCompletionStatus(iocpHandle, buffer->DataSize(), (ULONG_PTR)CompletionKeyType::CK_LOOPBACK, reinterpret_cast<LPOVERLAPPED>(iocpEvent));
	if (ret == false)
	{
		int32 errCode = GetLastError();
		std::cout << "[FAIL] PQCS ErrorCode: " << errCode << std::endl;
		return false;
	}
	return true;
}

bool IocpManager::PostDBResult(DBMessage* message)
{
	if (FALSE == PostQueuedCompletionStatus(iocpHandle, 0, (ULONG_PTR)CK_DB, (LPOVERLAPPED)message))
	{
		printf_s("IocpManager::PostDatabaseResult PostQueuedCompletionStatus Error: %d\n", GetLastError());
		CRASH_ASSERT(false);
	}

	return true;
}
