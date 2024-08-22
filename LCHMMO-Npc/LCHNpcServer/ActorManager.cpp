#include "pch.h"
#include "ActorManager.h"

std::unordered_map<uint32, ActorPtr> ActorManager::ActorIDToActorHashMap;
ActorManager GActorManager;