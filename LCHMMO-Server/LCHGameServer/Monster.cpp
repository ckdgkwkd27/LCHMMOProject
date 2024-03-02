#include "pch.h"
#include "Monster.h"
#include "ZoneManager.h"
#include "RandomUtil.h"
#include "Player.h"
#include "ClientPacketHandler.h"
#include "TimeUtil.h"

Monster::Monster()
{
	ActorInfo.set_objecttype((uint32)ObjectType::MONSTER);
	ActorInfo.set_actorid(GZoneManager.IssueActorID());
	ActorInfo.set_name("Monster" + std::to_string(ActorInfo.actorid()));
	ActorInfo.mutable_posinfo()->set_state((uint32)MoveState::IDLE);
	ActorInfo.mutable_statinfo()->set_level(1);
	ActorInfo.mutable_statinfo()->set_hp(10);
	ActorInfo.mutable_statinfo()->set_maxhp(10);
	ActorInfo.mutable_statinfo()->set_attack(5);
	ActorInfo.mutable_statinfo()->set_speed(2);
	ActorInfo.mutable_statinfo()->set_totalexp(0);
	RoamRadius = 2;
}

void Monster::Update()
{
	switch ((MoveState)ActorInfo.posinfo().state())
	{
	case MoveState::IDLE:
		UpdateIdle();
		break;
	case MoveState::MOVING:
		UpdateMoving();
		break;
	case MoveState::CHASING:
		UpdateChasing();
		break;
	case MoveState::SKILL:
		UpdateSkill();
		break;
	case MoveState::DEAD:
		UpdateDead();
		break;
	}
}

void Monster::UpdateIdle()
{
	if (nextIdleTimeStamp > CURRENT_TIMESTAMP().count())
		return;
	nextIdleTimeStamp = CURRENT_TIMESTAMP().count() + TICK_INTERVAL_IDLE;
	SetMoveState(MoveState::MOVING);
}

void Monster::UpdateMoving()
{
	if (nextMoveTimeStamp > CURRENT_TIMESTAMP().count())
		return;

	int64 _moveTick = static_cast<int64>(1000 / ActorInfo.statinfo().speed());
	nextMoveTimeStamp = CURRENT_TIMESTAMP().count() + _moveTick;

	ZonePtr zone = GZoneManager.FindZoneByID(zoneID);
	if (zone == nullptr)
	{
		std::cout << "[FAILURE] Monster ActorID=" << ActorInfo.actorid() << " Wrong ZoneID=" << zoneID << std::endl;
		CRASH_ASSERT(false);
	}

	PlayerPtr player = zone->FindPlayerInCondition([this](ActorPtr actor)
	{
		Vector2Int dir(ActorInfo.posinfo().posx() - actor->ActorInfo.posinfo().posx(),
			ActorInfo.posinfo().posy() - actor->ActorInfo.posinfo().posy());
		return sqrt(dir.CellDistFromZero) <= SEARCH_CELL_DISTANCE;
	});

	if (player != nullptr)
	{
		targetActor = player;
		SetMoveState(MoveState::CHASING);
		return;
	}

	Vector2Int CellPos(this->ActorInfo.posinfo().posx(), this->ActorInfo.posinfo().posy());

	//Patrol
	float Angle = RandomUtil::GetRandomFloat() * 360.0f;
	int32 TargetPositionX = this->ActorInfo.posinfo().posx() + (int32)(RoamRadius * cos(Angle));
	int32 TargetPositionY = this->ActorInfo.posinfo().posy() + (int32)(RoamRadius * sin(Angle));
	Vector2Int targetPos(TargetPositionX, TargetPositionY);

	if(zone->zoneMap.ApplyMove(shared_from_this(), targetPos))
		this->ActorInfo.mutable_posinfo()->set_movedir((uint32)GetDirFromVector(targetPos));
	BroadcastMove();
	SetMoveState(MoveState::IDLE);
}

void Monster::UpdateChasing()
{
	if (nextChaseTimeStamp > CURRENT_TIMESTAMP().count())
		return;

	int64 _moveTick = static_cast <int64>(1000 / ActorInfo.statinfo().speed());
	nextChaseTimeStamp = CURRENT_TIMESTAMP().count() + _moveTick;

	ZonePtr zone = GZoneManager.FindZoneByID(zoneID);
	CRASH_ASSERT(zone != nullptr);

	targetActor = zone->FindPlayerInCondition([this](ActorPtr actor)
	{
		Vector2Int dir(ActorInfo.posinfo().posx() - actor->ActorInfo.posinfo().posx(),
		ActorInfo.posinfo().posy() - actor->ActorInfo.posinfo().posy());
		return sqrt(dir.CellDistFromZero) <= SEARCH_CELL_DISTANCE;
	});

	if (targetActor == nullptr)
	{
		SetMoveState(MoveState::IDLE);
		return;
	}

	Vector2Int CellPos(this->ActorInfo.posinfo().posx(), this->ActorInfo.posinfo().posy());
	Vector2Int TrgCellPos(targetActor->ActorInfo.posinfo().posx(), targetActor->ActorInfo.posinfo().posy());

	Vector2Int dir = CellPos - TrgCellPos;
	int32 dist = static_cast<int32>(sqrt(dir.CellDistFromZero));
	if (dist == 0 || dist > CHASE_CELL_DISTANCE)
	{
		targetActor = nullptr;
		SetMoveState(MoveState::IDLE);
		return;
	}

	std::vector<Vector2Int> path = zone->zoneMap.FindPath(CellPos, TrgCellPos, false);
	if (path.size() < 2 || path.size() > CHASE_CELL_DISTANCE)
	{
		targetActor = nullptr;
		SetMoveState(MoveState::IDLE);
		return;
	} 

	if (dist <= SKILL_DISTANCE && (dir.x == 0 || dir.y == 0))
	{
		SetMoveState(MoveState::SKILL);
		return;
	}

	if (zone->zoneMap.ApplyMove(shared_from_this(), path[1]) == false)
	{
		SetMoveState(MoveState::MOVING);
		return;
	}

	this->ActorInfo.mutable_posinfo()->set_movedir((uint32)GetDirFromVector(path[1] - CellPos));
	BroadcastMove();
	SetMoveState(MoveState::CHASING);
}

void Monster::UpdateSkill()
{
	if (nextSkillTimeStamp > CURRENT_TIMESTAMP().count())
		return;

	nextSkillTimeStamp = CURRENT_TIMESTAMP().count() + TICK_INTERVAL_SKILL;

	ZonePtr zone = GZoneManager.FindZoneByID(targetActor->zoneID);
	CRASH_ASSERT(zone != nullptr);

	targetActor = zone->FindPlayerInCondition([this](ActorPtr actor)
	{
		Vector2Int dir(ActorInfo.posinfo().posx() - actor->ActorInfo.posinfo().posx(),
		ActorInfo.posinfo().posy() - actor->ActorInfo.posinfo().posy());
		return sqrt(dir.CellDistFromZero) <= SEARCH_CELL_DISTANCE;
	});

	if (targetActor == nullptr)
	{
		SetMoveState(MoveState::IDLE);
		return;
	}

	Vector2Int dir(targetActor->ActorInfo.posinfo().posx() - this->ActorInfo.posinfo().posx(),
		targetActor->ActorInfo.posinfo().posy() - this->ActorInfo.posinfo().posy());
	int32 dist = static_cast<int32>(sqrt(dir.CellDistFromZero));
	bool canUseSkill = (dist <= SKILL_DISTANCE && (dir.x == 0 || dir.y == 0));
	if (canUseSkill == false)
	{
		SetMoveState(MoveState::MOVING);
		return;
	}

	MoveDir	lookDir = GetDirFromVector(dir);
	if (ActorInfo.posinfo().movedir() != (uint32)lookDir)
	{
		ActorInfo.mutable_posinfo()->set_movedir((uint32)lookDir);
		BroadcastMove();
	}

	targetActor->OnDamaged(shared_from_this(), ActorInfo.statinfo().attack());

	protocol::ReturnSkill skillPacket;
	skillPacket.set_actorid(ActorInfo.actorid());
	skillPacket.set_skillid(0);
	auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(skillPacket);
	zone->BroadCast(shared_from_this(), _sendBuffer);

	//update cooltime
	int32 coolTick = 1000;
	nextSkillTimeStamp += coolTick;

	SetMoveState(MoveState::SKILL);
	BroadcastMove();
}

void Monster::UpdateDead()
{

}

void Monster::SetMoveState(MoveState _state)
{
	ActorInfo.mutable_posinfo()->set_state((uint32)_state);
}

void Monster::BroadcastMove()
{
	ZonePtr zone = GZoneManager.FindZoneByID(zoneID);
	CRASH_ASSERT(zone != nullptr);

	protocol::ReturnMove movePacket;
	movePacket.set_actorid(ActorInfo.actorid());
	movePacket.mutable_posinfo()->CopyFrom(ActorInfo.posinfo());
	//std::cout << "Current State=" << ActorInfo.posinfo().state() << std::endl;

	auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(movePacket);
	zone->BroadCast(shared_from_this(), _sendBuffer);

	SectionPtr section = zone->GetSection(Vector2Int::GetVectorFromActorPos(this->ActorInfo.posinfo()));
	section->PlayerViewportUpdate();
}