#pragma once
#include "NpcActor.h"

class ActorManager
{
public:
	static std::unordered_map<uint32, ActorPtr> ActorIDToActorHashMap;
	static RecursiveMutex managerLock;
};

extern ActorManager GActorManager;

#define SET_MANAGER_LOCK RecursiveLockGuard guard(GActorManager.managerLock);