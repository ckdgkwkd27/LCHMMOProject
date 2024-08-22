#include "pch.h"
#include "MapManager.h"
MapManager GMapManager;

bool MapManager::Init()
{
	for (uint32 idx = 0; idx < MAX_MAP_COUNT; idx++)
	{
		MapPtr _map = MACRO_NEW(Map);
		_map->LoadMap(idx);
		Maps.push_back(_map);
	}

	return true;
}

MapPtr MapManager::FindMap(uint32 zoneId)
{
	if (zoneId > MAX_MAP_COUNT)
	{
		return nullptr;
	}

	return Maps[zoneId];
}
