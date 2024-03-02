#include "pch.h"
#include "ZoneManager.h"
#include "ClientPacketHandler.h"
#include "RandomUtil.h"

ZoneManager GZoneManager;

ZoneManager::ZoneManager()
{
	zonePool = std::make_shared<ObjectPool<Zone>>(10);
	numOfActors = 0;
}

void ZoneManager::Initialize()
{
	for (uint32 idx = 0; idx < ZoneIDEnum::CNT; idx++)
	{
		ZonePtr _zone = ZonePtr(zonePool->BorrowObject());
		_zone->zoneID = idx;
		_zone->Init();
		RegisterZone(_zone);
	}

	std::cout << "[INFO] Zone Init Completed!" << std::endl;
}

void ZoneManager::RegisterZone(ZonePtr _zone)
{
	RecursiveLockGuard guard(zoneLock);
	zoneVector.push_back(_zone);
}

bool ZoneManager::RegisterActor(ZoneIDType zoneId, ActorPtr _actor)
{
	RecursiveLockGuard guard(zoneLock);

	ZonePtr _zone = nullptr;
	auto it = zoneVector.begin();
	for (; it != zoneVector.end(); it++)
	{
		if ((*it)->zoneID == zoneId)
		{
			_zone = *it;
			break;
		}
	}

	//그런 존이 없음
	if (it == zoneVector.end())
	{
		return false;
	}

	//이미 액터가 존재
	if (nullptr != (*it)->FindActor(_actor->ActorInfo.actorid()))
	{
		return false;
	}
	
	(*it)->RegisterActor(_actor);
	return true;
}

ActorIDType ZoneManager::IssueActorID()
{
	RecursiveLockGuard guard(zoneLock);
	uint32 newActorID = numOfActors;
	numOfActors++;
	return newActorID;
}

ZonePtr ZoneManager::FindZoneByID(ZoneIDType _zoneId)
{
	//RecursiveLockGuard guard(zoneLock);
	auto it = std::find_if(zoneVector.begin(), zoneVector.end(), [_zoneId](ZonePtr _zone) { return _zone->zoneID == _zoneId; });
	if(it == zoneVector.end())
		return nullptr;
	return *it;
}

void ZoneManager::TickUpdate()
{
	CurrTimeStamp = TimeUtil::GetCurrTimeStamp();

	//존 업데이트
	if (CurrTimeStamp > NextTimeStampForZoneUpdate)
	{
		for (ZonePtr _zone : zoneVector)
			_zone->Update();

		NextTimeStampForZoneUpdate = CurrTimeStamp + ZONE_UPDATE_INTERVAL;
	}
}

void ZoneManager::SpawnNpc()
{
	RecursiveLockGuard guard(zoneLock);

	//Spawn Monster or Npc
	for (ZonePtr _zone : zoneVector)
	{
		if (_zone->zoneID == 1)
		{
			for (uint32 i = 0; i < 100; i++)
			{
				MonsterPtr monster = std::make_shared<Monster>();
				monster->zoneID = _zone->zoneID;
				monster->ActorInfo.mutable_posinfo()->set_posx(RandomUtil::GetRandomRangeInt(_zone->xMin, _zone->xMax) / 2);
				monster->ActorInfo.mutable_posinfo()->set_posy(RandomUtil::GetRandomRangeInt(_zone->yMin, _zone->yMax) / 2);
				_zone->RegisterActor(monster);
			}

			std::cout << "[INFO] ZoneID=" << _zone->zoneID << " Monster Spawned!" << std::endl;
		}
	}
}
