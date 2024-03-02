#pragma once
#include "SocketUtil.h"
#include "SessionManager.h"
#include "IocpEvent.h"
#include "DBMessage.h"

enum CompletionKeyType
{
	CK_IOCP,
	CK_DB,
	CK_LOOPBACK,
};

class IocpManager
{
public:
	IocpManager(std::wstring _ip, uint16 _port, uint32 _maxConnectionCnt);
	
	void Initialize();
	void BindAndListen();
	bool StartWorker();
	bool Join();

	template <typename T>
	void StartConnect();
	
	template <typename T>
	void StartAccept();

	void Dispatch(IocpEvent* iocpEvent, DWORD bytes);
	void AcceptThreadFunc();
	void WorkerThreadFunc();
	bool IocpPost(CircularBufferPtr buffer, IocpEvent* iocpEvent);
	bool PostDBResult(DBMessage* message);

private:
	std::wstring ip;
	uint16 port;
	uint32 maxConnectionCnt;
	SOCKET listenSocket;
	HANDLE iocpHandle;
	uint32 workerThreadsCnt;
	std::thread AcceptThread;
	std::vector<std::thread> workerThreads;

public:
	std::atomic<uint64> sendTps = 0;
	std::atomic<uint64> recvTps = 0;

public:
	HANDLE GetIocpHandle() { return iocpHandle; }
	SOCKET GetListenSocket() { return listenSocket; }
	uint32 GetMaxConnectionCnt() { return maxConnectionCnt; }
};

extern IocpManager* GIocpManager;

template <typename T>
void IocpManager::StartAccept()
{
	GSessionManager.PrepareSessions<T>(maxConnectionCnt, listenSocket, iocpHandle);

	AcceptThread = std::thread(&IocpManager::AcceptThreadFunc, this);
	std::cout << "[INFO] Start Accept..." << std::endl;
}

template <typename T>
void IocpManager::StartConnect()
{
	GSessionManager.PrepareSessions<T>(maxConnectionCnt, INVALID_SOCKET, iocpHandle);
	GSessionManager.ConnectServerSession(maxConnectionCnt, ip, port);
	std::cout << "[INFO] Start Connect..." << std::endl;
}