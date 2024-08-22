#pragma once
#include "Session.h"

class WorldSession : public Session
{
public:
	virtual void OnAccepted();
	virtual void OnConnected();
	virtual void OnDisconnected();
	virtual uint32 OnRecv(char* buffer, uint32 len);

private:
	const uint32 serviceId = 2;
	static uint32 GlobalNpcIDCnt;
};

using WorldSessionPtr = std::shared_ptr<WorldSession>;
