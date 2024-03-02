#pragma once
class SocketUtil
{
public:
	static LPFN_CONNECTEX ConnectEx;
	static LPFN_DISCONNECTEX DisconnectEx;

	static bool Init();
	static void Clear();
	static SOCKET CreateSocket();
	static void CloseSocket(SOCKET& _socket);

	static bool Bind(SOCKET _socket, Wstring ip, uint16 port);
	static bool BindAnyAddress(SOCKET _socket, uint16 port);
	static bool Listen(SOCKET _socket, int32 backlog = SOMAXCONN);

	static bool SetOptionLinger(SOCKET _socket);
	static bool SetOptionReuseAddr(SOCKET _socket, bool flag);
	static bool SetOptionNoDelay(SOCKET _socket, bool flag);
	static bool SetOptionUpdateAcceptSocket(SOCKET _socket, SOCKET listenSocket);
	static bool SetOptionKeepAlive(SOCKET _socket, bool flag);

	static SOCKET CreateListenSocket();
};

