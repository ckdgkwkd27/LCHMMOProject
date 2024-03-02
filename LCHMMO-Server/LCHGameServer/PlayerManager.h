#pragma once
#define MAX_PLAYER 65535
#include "Player.h"

class PlayerManager {
public:
	void Initialize(uint64 maxSize = MAX_PLAYER);
	PlayerPtr NewPlayer();
	void DeletePlayer(PlayerPtr _player);
	PlayerPtr FindPlayerByID(PlayerIDType playerID);
	PlayerPtr FindPlayerByName(std::string _name);

private:
	void AddPlayer(PlayerPtr _player);
	void ReturnPlayer(PlayerPtr _player);

public:
	std::unordered_map<uint64, PlayerPtr> AllPlayerInfo;

private:
	RecursiveMutex playerLock;
	std::queue<PlayerPtr> playerPool;
	uint64 numOfPlayers = 0;
	uint64 playerIdx = 0;

public:
	uint64 GetPlayerIdx() { return playerIdx; }
	void IncreasePlayerIdx() { playerIdx++; }
	void DecreasePlayerIdx() { playerIdx--; }
};

extern PlayerManager GPlayerManager;

