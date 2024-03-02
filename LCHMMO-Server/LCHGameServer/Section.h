#pragma once
#include "Player.h"

class Section
{
public:
	Section(int32 x, int32 y);
	void Add(ActorPtr actor);
	void Remove(ActorPtr actor);
	ActorPtr FindActor(ActorIDType _actorId);
	PlayerPtr FindPlayerInCondition(std::function<bool(ActorPtr)> _condition);
	bool PlayerViewportUpdate();

private:
	RecursiveMutex sectionLock;
	int indexX, indexY;
public:
	std::vector<ActorPtr> actorVector;
};

using SectionPtr = std::shared_ptr<Section>;