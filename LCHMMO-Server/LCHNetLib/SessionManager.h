#pragma once
#include "Session.h"
#include "CircularBuffer.h"

class IocpManager;

class SessionManager
{
public:
	std::list<SessionPtr> activeSessions;
	std::list<SessionPtr> sessionPool;

private:
	RecursiveMutex sessionLock;

	uint32 userCnt = 0; //À¯Àú ¼ö
	uint32 issueCnt = 0;

public:
	void Broadcast(CircularBufferPtr _sendBuffer);

	bool AcceptClientSession(uint32 maxSessionCnt);
	bool ConnectServerSession(uint32 maxSessionCnt, Wstring connIp, uint16 connPort);

	template <typename T>
	void PrepareSessions(uint32 maxSessionCnt, SOCKET _listenSocket, HANDLE _iocpHandle);
	SessionPtr IssueSession();
	void ReturnSession(SessionPtr _session);

	void AddToActivePool(SessionPtr _session);
	void DeleteFromActivePool(SessionPtr _session);

	uint32 GetIssueCount() { return issueCnt; }
	std::list<SessionPtr> GetSessionPool() { return sessionPool; }
};

extern SessionManager GSessionManager;

template <typename T>
void SessionManager::PrepareSessions(uint32 maxSessionCnt, SOCKET _socket, HANDLE _iocpHandle)
{
	for (uint32 i = 0; i < maxSessionCnt; i++)
	{
		SessionPtr _session = std::make_shared<T>();
		_session->SetListenSocket(_socket);
		_session->SetIocpHandle(_iocpHandle);
		sessionPool.push_back(_session);
	}
}