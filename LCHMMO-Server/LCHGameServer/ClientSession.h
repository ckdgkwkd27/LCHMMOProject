#pragma once
#include "Session.h"
#include "SessionManager.h"

enum class SessionState : uint8
{
	NONE,
	CONNECTED,
	LOGIN,
	ENTER_GAME,

	END
};

enum class SessionServiceType : uint8
{
	NONE,
	USER,
	NPC,

	END
};

class Player;
class ClientSession : public Session
{
public:
	virtual void OnAccepted();
	virtual void OnConnected();
	virtual void OnDisconnected();
	virtual uint32 OnRecv(char* buffer, uint32 len);

public:
	std::shared_ptr<Player> currentPlayer;
	SessionServiceType ServiceType;
};

using ClientSessionPtr = std::shared_ptr<ClientSession>;