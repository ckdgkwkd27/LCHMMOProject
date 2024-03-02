#pragma once
#include "Actor.h"
#include "Player.h"
#include "Map.h"
#include "MessageQueue.h"
#include "ClientPacketHandler.h"
#include "Section.h"
#define SECTION_SIZE 10
#define VIEWPORT_CELL 10

using ZoneIDType = uint32;
class Zone
{
public:
	void Init();
	void RegisterActor(ActorPtr _actor);
	void UnregisterActor(ActorPtr _actor);
	ActorPtr FindActor(ActorIDType _actorId);
	PlayerPtr FindPlayerInCondition(std::function<bool(ActorPtr)> _condition);

	SectionPtr GetSection(Vector2Int secionPos);
	SectionPtr GetSection(int32 indexY, uint32 indexX);
	std::vector<PlayerPtr> GetAdjacentPlayers(Vector2Int pos, uint32 range);
	std::set<SectionPtr> GetAdjacentSections(Vector2Int sectionPos, uint32 range = VIEWPORT_CELL);

	void BroadCast(ActorPtr _selfPlayer, CircularBufferPtr _sendBuffer);
	void BroadCast(Vector2Int cellPos, CircularBufferPtr _sendBuffer);
	bool Update();
	bool PlayerViewportUpdate();
	void EnterGame(PlayerPtr player, ZoneIDType zoneId = 0);
	void LeaveGame(ActorIDType _actorId);
	void HandleMove(PlayerPtr player, protocol::RequestMove movePacket);
	void HandleSkill(PlayerPtr player, protocol::RequestSkill packet);

public:
	ZoneIDType zoneID;
	std::vector<ActorPtr> actorVector;
	std::vector<std::vector<SectionPtr>> sectionVector;
	uint32 sectionCells;
	int32 xMax, xMin, yMax, yMin;
	Map zoneMap;
	MessageQueue messageQueue;

private:
	RecursiveMutex actorLock;
};

using ZonePtr = std::shared_ptr<Zone>;
