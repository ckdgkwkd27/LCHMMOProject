#pragma once
#include "Map.h"
#define MAX_MAP_COUNT 2

class MapManager
{
public:
	bool Init();
	MapPtr FindMap(uint32 zoneId);

private:
	std::vector<MapPtr> Maps;
};

extern MapManager GMapManager;