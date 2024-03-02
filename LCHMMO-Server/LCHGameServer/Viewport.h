#pragma once
#include "Player.h"
class Viewport
{
public:
	Viewport(PlayerPtr _owner);
	std::set<ActorPtr> GatherActors();
	void Update();

public:
	PlayerPtr owner;
	std::set<ActorPtr> prevActors;
};

using ViewportPtr = std::shared_ptr<Viewport>;