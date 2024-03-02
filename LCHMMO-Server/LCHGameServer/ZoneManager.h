#pragma once
#include "Zone.h"
#include "ObjectPool.h"
#include "Monster.h"
#include "TimeUtil.h"
#define ZONE_UPDATE_INTERVAL 60ms
#define PLAYER_VIEWPORT_UPDATE_INTERVAL 100ms

class ClientPacketHandler;

enum ZoneIDEnum
{
	START_ZONE,
	FIELD_ZONE,

	CNT
};

class ZoneManager
{
public:
	ZoneManager();
	void Initialize();
	void RegisterZone(ZonePtr _zone);
	bool RegisterActor(ZoneIDType _zoneId, ActorPtr _actor);
	ActorIDType IssueActorID();
	ZonePtr FindZoneByID(ZoneIDType _zoneId);
	void TickUpdate();

public:
	std::vector<ZonePtr> zoneVector;
	std::shared_ptr<ObjectPool<Zone>> zonePool;
	uint32 numOfActors;

public:
	RecursiveMutex zoneLock;
	void SpawnNpc();

private:
	TimeStampType CurrTimeStamp = ZERO_TIMESTAMP;
	TimeStampType NextTimeStampForZoneUpdate = ZERO_TIMESTAMP;
	TimeStampType NextTimeStampForPlayerViewportUpdate = ZERO_TIMESTAMP;
};

extern ZoneManager GZoneManager;
