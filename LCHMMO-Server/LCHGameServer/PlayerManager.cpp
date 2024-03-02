#include "pch.h"
#include "PlayerManager.h"
#include "ZoneManager.h"
PlayerManager GPlayerManager;

void PlayerManager::Initialize(uint64 maxSize)
{
	for (uint32 idx = 0; idx < maxSize; idx++)
	{
		PlayerPtr _player = std::make_shared<Player>();
		_player->playerId = idx;
		playerPool.push(_player);
	}
	numOfPlayers = 0;

	std::cout << "[INFO] Player Init Completed!" << std::endl;
}

PlayerPtr PlayerManager::NewPlayer()
{
	RecursiveLockGuard playerGuard(playerLock);
	PlayerPtr _player = playerPool.front();
	playerPool.pop();
	AddPlayer(_player);
	IncreasePlayerIdx();
	return _player;
}

void PlayerManager::AddPlayer(PlayerPtr _player)
{
	AllPlayerInfo.insert({playerIdx, _player});
	numOfPlayers++;
}

void PlayerManager::DeletePlayer(PlayerPtr _player)
{
	RecursiveLockGuard playerGuard(playerLock);
	uint64 _playerId = _player->playerId;
	AllPlayerInfo.erase(_playerId);
	ReturnPlayer(_player);
}

PlayerPtr PlayerManager::FindPlayerByID(PlayerIDType playerID)
{
	for (auto& info : AllPlayerInfo)
	{
		if (info.second->playerId == playerID)
		{
			return info.second;
		}
	}
	return nullptr;
}

PlayerPtr PlayerManager::FindPlayerByName(std::string _name)
{
	for (auto& info : AllPlayerInfo)
	{
		if (info.second->ActorInfo.name() == _name)
		{
			return info.second;
		}
	}
	return nullptr;
}


void PlayerManager::ReturnPlayer(PlayerPtr _player)
{
	playerPool.push(_player);
	numOfPlayers--;
}
