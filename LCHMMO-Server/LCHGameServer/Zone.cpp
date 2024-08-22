#include "pch.h"
#include "Zone.h"
#include "ClientSession.h"
#include "ZoneManager.h"
#include "PlayerManager.h"
#include "Viewport.h"
#include "IocpManager.h"

void Zone::Init()
{
	zoneMap.LoadMap(zoneID);
	this->xMax = zoneMap.MaxX;
	this->xMin = zoneMap.MinX;
	this->yMax = zoneMap.MaxY;
	this->yMin = zoneMap.MinY;

	uint32 SizeX = xMax - xMin + 1;
	uint32 SizeY = yMax - yMin + 1;

	sectionCells = SECTION_SIZE;
	int32 countY = (SizeY + sectionCells - 1) / sectionCells;
	int32 countX = (SizeX + sectionCells - 1) / sectionCells;
	sectionVector.resize(countY + 1, std::vector<SectionPtr>(countX + 1));
	for (int32 y = 0; y <= countY; y++)
	{
		for (int32 x = 0; x <= countX; x++)
		{
			sectionVector[y][x] = std::make_shared<Section>(x, y);
		}
	}
}

void Zone::RegisterActor(ActorPtr _actor)
{
	if (_actor == nullptr)
		return;

	RecursiveLockGuard guard(actorLock);
	actorVector.push_back(_actor);
}

void Zone::UnregisterActor(ActorPtr _actor)
{
	if (_actor == nullptr)
		return;

	actorVector.erase(std::remove(actorVector.begin(), actorVector.end(), _actor), actorVector.end());
}

ActorPtr Zone::FindActor(ActorIDType _actorID)
{
	RecursiveLockGuard guard(actorLock);

	auto it = std::find_if(actorVector.begin(), actorVector.end(), [_actorID](ActorPtr _actor) {
		if (_actor == nullptr) CRASH_ASSERT(false);
		return _actor->ActorInfo.actorid() == _actorID;
	});

	if (it == actorVector.end())
		return nullptr;

	return *it;
}

PlayerPtr Zone::FindPlayerInCondition(std::function<bool(ActorPtr)> _condition)
{
	RecursiveLockGuard guard(actorLock);

	for (ActorPtr _actor : actorVector)
	{
		if (_actor->ActorInfo.objecttype() == (uint32)ObjectType::PLAYER)
		{
			if (_condition(_actor) == true)
			{
				return std::dynamic_pointer_cast<Player>(_actor);
			}
		}
	}
	return nullptr;
}

SectionPtr Zone::GetSection(Vector2Int sectionPos)
{
	int32 x = (sectionPos.x - zoneMap.MinX) / sectionCells;
	int32 y = (zoneMap.MaxY - sectionPos.y) / sectionCells;
	return GetSection(y, x);
}

SectionPtr Zone::GetSection(int32 indexY, uint32 indexX)
{
	if (indexX < 0 || indexX >= sectionVector[1].size())
		return nullptr;
	if (indexY < 0 || indexY >= sectionVector[0].size())
		return nullptr;

	return sectionVector[indexY][indexX];
}

std::vector<PlayerPtr> Zone::GetAdjacentPlayers(Vector2Int pos, uint32 range)
{
	std::set<SectionPtr> sections = GetAdjacentSections(pos, range);

	std::vector<PlayerPtr> players;
	for (SectionPtr _section : sections)
	{
		for (ActorPtr _actor : _section->actorVector)
		{
			if (_actor->ActorInfo.objecttype() == (uint32)ObjectType::PLAYER)
				players.push_back(std::dynamic_pointer_cast<Player>(_actor));
		}
	}

	return players;
}

std::set<SectionPtr> Zone::GetAdjacentSections(Vector2Int sectionPos, uint32 range)
{
	std::set<SectionPtr> sections;
	int32 maxY = sectionPos.y + range;
	int32 maxX = sectionPos.x + range;
	int32 minY = sectionPos.y - range;
	int32 minX = sectionPos.x - range;

	Vector2Int leftTop(minX, maxY);
	int32 minIndexY = (zoneMap.MaxY - leftTop.y) / sectionCells;
	int32 minIndexX = (leftTop.x - zoneMap.MinX) / sectionCells;

	Vector2Int rightBottom(maxX, minY);
	int32 maxIndexY = (zoneMap.MaxY - rightBottom.y) / sectionCells;
	int32 maxIndexX = (rightBottom.x - zoneMap.MinX) / sectionCells;

	for (int32 x = minIndexX; x <= maxIndexX; x++)
	{
		for (int32 y = minIndexY; y <= maxIndexY; y++)
		{
			SectionPtr _section = GetSection(y, x);
			if (_section == nullptr)
				continue;

			sections.insert(_section);
		}
	}

	return sections;
}

void Zone::BroadCast(ActorPtr _selfPlayer, CircularBufferPtr _sendBuffer)
{
	//RecursiveLockGuard guard(actorLock);
	Vector2Int posCell(_selfPlayer->ActorInfo.posinfo().posx(), _selfPlayer->ActorInfo.posinfo().posy());
	std::set<SectionPtr> sections = GetAdjacentSections(posCell);

	for (auto _section : sections)
	{
		for (auto _actor : _section->actorVector)
		{
			if (_actor->ActorInfo.objecttype() == (uint32)ObjectType::PLAYER && _actor != nullptr && _actor != _selfPlayer)
			{
				Vector2Int playerCellPos(_actor->ActorInfo.posinfo().posx(), _actor->ActorInfo.posinfo().posy());
				int32 dx = playerCellPos.x - posCell.x;
				int32 dy = playerCellPos.y - posCell.y;
				if(abs(dx) > VIEWPORT_CELL)
					continue;
				if (abs(dy) > VIEWPORT_CELL)
					continue;

				PlayerPtr _player = std::dynamic_pointer_cast<Player>(_actor);
				_player->ownerSession->PostSend(_sendBuffer);

				//Viewport Update
				//_player->viewport->Update();
			}
		}
	}
}

void Zone::BroadCast(Vector2Int cellPos, CircularBufferPtr _sendBuffer)
{
	std::set<SectionPtr> sections = GetAdjacentSections(cellPos);
	for (auto _section : sections)
	{
		for (auto _actor : _section->actorVector)
		{
			if (_actor->ActorInfo.objecttype() == (uint32)ObjectType::PLAYER && _actor != nullptr)
			{
				Vector2Int playerCellPos(_actor->ActorInfo.posinfo().posx(), _actor->ActorInfo.posinfo().posy());
				int32 dx = playerCellPos.x - cellPos.x;
				int32 dy = playerCellPos.y - cellPos.y;
				if (abs(dx) > VIEWPORT_CELL)
					continue;
				if (abs(dy) > VIEWPORT_CELL)
					continue;

				PlayerPtr _player = std::dynamic_pointer_cast<Player>(_actor);
				_player->ownerSession->PostSend(_sendBuffer);
			}
		}
	}
}

bool Zone::Update()
{
	//RecursiveLockGuard guard(actorLock);

	//Actor Update
	for (ActorPtr _actor : actorVector)
	{
		if(_actor != nullptr)
			_actor->Update();
	}

	//Message Queue Process
	std::vector<MessageFuncType> messageVector;
	messageQueue.FlushAll(messageVector);
	for (MessageFuncType message : messageVector)
	{
		message();
	}

	return true;
}

void Zone::EnterGame(PlayerPtr player, ZoneIDType zoneId)
{
	if (zoneId < 0 || zoneId >= ZoneIDEnum::CNT)
		return;

	if (false == GZoneManager.RegisterActor(zoneId, player))
		return;

	Vector2Int cellPos = Vector2Int::GetVectorFromActorPos(player->ActorInfo.posinfo());
	this->zoneMap.ApplyMove(player, cellPos);
	GetSection(cellPos)->Add(player);

	//Player 게임입장
	{
		protocol::ReturnEnterGame ReturnPkt;
		ReturnPkt.mutable_myplayer()->CopyFrom(player->ActorInfo);
		ReturnPkt.set_zoneid(player->zoneID);
		auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(ReturnPkt);
		player->ownerSession->PostSend(_sendBuffer);
	}

	SectionPtr section = GetSection(cellPos);
	section->PlayerViewportUpdate();

	printf("INFO: EnterGame ActorID=%d, ZoneID=%d\n", player->ActorInfo.actorid(), zoneID);
}

void Zone::LeaveGame(ActorIDType _actorId)
{
	auto actor = FindActor(_actorId);
	if (actor == nullptr)
		return;

	if (actor->ActorInfo.objecttype() == (uint32)ObjectType::PLAYER)
	{
		this->UnregisterActor(actor);

		PlayerPtr player = std::dynamic_pointer_cast<Player>(actor);
		GPlayerManager.DeletePlayer(player);

		zoneMap.ApplyLeave(actor);

		protocol::NotifyDespawn despawnPacket;
		auto sendBuffer = ClientPacketHandler::MakeSendBufferPtr(despawnPacket);
		player->ownerSession->PostSend(sendBuffer);
	}

	//BroadCast
	{
		protocol::NotifyDespawn despawnPacket;
		despawnPacket.add_actorids(_actorId);
		auto sendBuffer = ClientPacketHandler::MakeSendBufferPtr(despawnPacket);
		BroadCast(Vector2Int::GetVectorFromActorPos(actor->ActorInfo.posinfo()), sendBuffer);
	}
}

void Zone::HandleMove(ActorPtr actor, protocol::RequestMove movePacket)
{
	if (actor == nullptr)
		return;

	//Verify
	protocol::PositionInfo movePosInfo = movePacket.posinfo();
	if (movePosInfo.posx() != actor->ActorInfo.posinfo().posx() || movePosInfo.posy() != actor->ActorInfo.posinfo().posy())
	{
		if (zoneMap.CanGo(Vector2Int(movePosInfo.posx(), movePosInfo.posy())) == false)
			return;
	}

	actor->ActorInfo.mutable_posinfo()->set_state(movePosInfo.state());
	actor->ActorInfo.mutable_posinfo()->set_movedir(movePosInfo.movedir());
	Vector2Int moveCellPos = Vector2Int(movePosInfo.posx(), movePosInfo.posy());
	zoneMap.ApplyMove(actor, moveCellPos);

	//BroadCast
	protocol::ReturnMove resMovePacket;
	resMovePacket.set_actorid(actor->ActorInfo.actorid());
	resMovePacket.mutable_posinfo()->CopyFrom(movePosInfo);
	auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(resMovePacket);
	BroadCast(actor, _sendBuffer);

	SectionPtr section = GetSection(moveCellPos);
	section->PlayerViewportUpdate();

	//npc에게 Send
	if (actor->ActorInfo.objecttype() == (uint32)ObjectType::PLAYER && actor->zoneID == 1)
	{
		this->messageQueue.Push([actor, movePosInfo]
			{
				for (auto _session : GSessionManager.activeSessions)
				{
					ClientSessionPtr clientSession = std::static_pointer_cast<ClientSession>(_session);
					if (clientSession->ServiceType == SessionServiceType::NPC)
					{
						protocol::ReturnMove movePacket;
						movePacket.set_actorid(actor->ActorInfo.actorid());
						movePacket.mutable_posinfo()->CopyFrom(movePosInfo);
						auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(movePacket);
						_session->PostSend(_sendBuffer);
					}
				}
			}
		);
	}
}

void Zone::HandleSkill(ActorPtr actor, protocol::RequestSkill packet)
{
	if (actor == nullptr)
		return;

	double dist = 0.f;
	ActorPtr targetActor;

	//Check Distance
	{
		ZonePtr	_zone = GZoneManager.FindZoneByID(actor->zoneID);
		auto it = std::find_if(_zone->actorVector.begin(), _zone->actorVector.end(), [packet](ActorPtr _actor)
		{
			return _actor->ActorInfo.actorid() == packet.targetactorid();
		});

		if (it == _zone->actorVector.end())
			return;
		if ((*it)->ActorInfo.statinfo().hp() <= 0)
			return;

		targetActor = *it;
		Vector2Int targetPos(targetActor->ActorInfo.posinfo().posx(), targetActor->ActorInfo.posinfo().posy());
		Vector2Int actorPos(actor->ActorInfo.posinfo().posx(), actor->ActorInfo.posinfo().posy());
		dist = Vector2Int::Distance(targetPos, actorPos);
	}

	if (packet.skillid() == 2)
	{
		if (dist > 3.f)
			return;

		targetActor->OnDamaged(actor, actor->ActorInfo.statinfo().attack());
		printf("INFO: Player=%s Melee Atack to Monster=%s Damage=%d\n", actor->ActorInfo.name().c_str(), targetActor->ActorInfo.name().c_str(), actor->ActorInfo.statinfo().attack());
	}

	else if (packet.skillid() == 3)
	{

	}

	//BroadCast
	protocol::ReturnSkill resSkillPacket;
	resSkillPacket.set_actorid(actor->ActorInfo.actorid());
	resSkillPacket.set_skillid(packet.skillid());
	auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(resSkillPacket);
	BroadCast(Vector2Int::GetVectorFromActorPos(actor->ActorInfo.posinfo()), _sendBuffer);
}
