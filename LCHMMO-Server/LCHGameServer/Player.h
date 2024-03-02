#pragma once
#include "Actor.h"
using PlayerIDType = uint32;

class ClientSession;
class Viewport;

class Player : public Actor
{
public:
	PlayerIDType playerId;
	std::shared_ptr<ClientSession> ownerSession;
	std::shared_ptr<Viewport> viewport;

	virtual void ViewportUpdate();
};

using PlayerPtr = std::shared_ptr<Player>;
