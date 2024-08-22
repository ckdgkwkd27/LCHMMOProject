#include "pch.h"
#include "Monster.h"
#include "WorldPacketHandler.h"
#include "MapManager.h"
#include "RandomUtil.h"

Monster::Monster()
{
	ActorInfo.mutable_objects()->set_objecttype((uint32)ObjectType::MONSTER);
	ActorInfo.mutable_objects()->set_name("Monter");
	ActorInfo.mutable_objects()->mutable_posinfo()->set_state((uint32)MoveState::IDLE);
	ActorInfo.mutable_objects()->mutable_statinfo()->set_level(1);
	ActorInfo.mutable_objects()->mutable_statinfo()->set_hp(10);
	ActorInfo.mutable_objects()->mutable_statinfo()->set_maxhp(10);
	ActorInfo.mutable_objects()->mutable_statinfo()->set_attack(5);
	ActorInfo.mutable_objects()->mutable_statinfo()->set_speed(2);
	ActorInfo.mutable_objects()->mutable_statinfo()->set_totalexp(0);
	RoamRadius = 2;
}

void Monster::Update()
{
	switch ((MoveState)ActorInfo.mutable_objects()->posinfo().state())
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

	int64 _moveTick = static_cast<int64>(1000 / ActorInfo.mutable_objects()->statinfo().speed());
	nextMoveTimeStamp = CURRENT_TIMESTAMP().count() + _moveTick;

	for (auto p : GActorManager.ActorIDToActorHashMap)
	{
		ActorPtr player = p.second;
		if (player->ActorInfo.objects().objecttype() == static_cast<uint32>(ObjectType::PLAYER))
		{
			Vector2Int dir(ActorInfo.objects().posinfo().posx() - player->ActorInfo.objects().posinfo().posx(),
				this->ActorInfo.objects().posinfo().posy() - player->ActorInfo.objects().posinfo().posy());
			uint32 dist = static_cast<uint32>(sqrt(dir.CellDistFromZero));
			if (dist <= SEARCH_CELL_DISTANCE)
			{
				targetActor = player;
				SetMoveState(MoveState::CHASING);
				return;
			}
		}
	}

	Vector2Int CellPos(this->ActorInfo.objects().posinfo().posx(), this->ActorInfo.objects().posinfo().posy());

	//Patrol
	float Angle = RandomUtil::GetRandomFloat() * 360.0f;
	int32 TargetPositionX = this->ActorInfo.objects().posinfo().posx() + (int32)(RoamRadius * cos(Angle));
	int32 TargetPositionY = this->ActorInfo.objects().posinfo().posy() + (int32)(RoamRadius * sin(Angle));
	Vector2Int targetPos(TargetPositionX, TargetPositionY);

	this->ActorInfo.mutable_objects()->mutable_posinfo()->set_movedir((uint32)GetDirFromVector(targetPos));

	MapPtr _map = GMapManager.FindMap(1);
	RETURN_ON_FAIL(_map != nullptr);
	_map->ApplyMove(shared_from_this(), targetPos);

	BroadcastMove(this->ActorInfo.objects().actorid(), targetPos);
	SetMoveState(MoveState::IDLE);
}

void Monster::UpdateChasing()
{
	if (nextChaseTimeStamp > CURRENT_TIMESTAMP().count())
		return;

	int64 _moveTick = static_cast <int64>(1000 / ActorInfo.objects().statinfo().speed());
	nextChaseTimeStamp = CURRENT_TIMESTAMP().count() + _moveTick;

	//target은 유효해야 함
	if (targetActor == nullptr)
	{
		SetMoveState(MoveState::IDLE);
		return;
	}
	
	Vector2Int CellPos(this->ActorInfo.objects().posinfo().posx(), this->ActorInfo.objects().posinfo().posy());
	Vector2Int TrgCellPos(targetActor->ActorInfo.objects().posinfo().posx(), targetActor->ActorInfo.objects().posinfo().posy());

	Vector2Int dir = CellPos - TrgCellPos;
	int32 dist = static_cast<int32>(sqrt(dir.CellDistFromZero));
	if (dist == 0 || dist > CHASE_CELL_DISTANCE)
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

	MapPtr _map = GMapManager.FindMap(1);
	RETURN_ON_FAIL(_map != nullptr);

	std::vector<Vector2Int> path = _map->FindPath(CellPos, TrgCellPos, false);
	if (path.size() < 2 || path.size() > CHASE_CELL_DISTANCE)
	{
		targetActor = nullptr;
		SetMoveState(MoveState::IDLE);
		return;
	}

	if (_map->ApplyMove(shared_from_this(), path[1]) == false)
	{
		SetMoveState(MoveState::MOVING);
		return;
	}

	this->ActorInfo.mutable_objects()->mutable_posinfo()->set_movedir((uint32)GetDirFromVector(path[1] - CellPos));
	BroadcastMove(this->ActorInfo.objects().actorid(), TrgCellPos);
	SetMoveState(MoveState::CHASING);
}

void Monster::UpdateSkill()
{
	if (nextSkillTimeStamp > CURRENT_TIMESTAMP().count())
		return;

	nextSkillTimeStamp = CURRENT_TIMESTAMP().count() + TICK_INTERVAL_SKILL;

	//target은 유효해야 함
	if (targetActor == nullptr)
	{
		SetMoveState(MoveState::IDLE);
		BroadcastMove(this->ActorInfo.objects().actorid(), Vector2Int::GetVectorFromActorPos(this->ActorInfo.objects().posinfo()));
		return;
	}

	Vector2Int dir(targetActor->ActorInfo.objects().posinfo().posx() - this->ActorInfo.objects().posinfo().posx(),
		targetActor->ActorInfo.objects().posinfo().posy() - this->ActorInfo.objects().posinfo().posy());
	int32 dist = static_cast<int32>(sqrt(dir.CellDistFromZero));
	bool canUseSkill = (dist <= SKILL_DISTANCE && (dir.x == 0 || dir.y == 0));
	if (canUseSkill == false)
	{
		SetMoveState(MoveState::MOVING);
		BroadcastMove(this->ActorInfo.objects().actorid(), Vector2Int::GetVectorFromActorPos(this->ActorInfo.objects().posinfo()));
		return;
	}

	MoveDir	lookDir = GetDirFromVector(dir);
	if (ActorInfo.objects().posinfo().movedir() != (uint32)lookDir)
	{
		ActorInfo.mutable_objects()->mutable_posinfo()->set_movedir((uint32)lookDir);
		BroadcastMove(this->ActorInfo.objects().actorid(), Vector2Int::GetVectorFromActorPos(this->ActorInfo.objects().posinfo()));
	}

	targetActor->OnDamaged(shared_from_this(), ActorInfo.objects().statinfo().attack());

	protocol::RequestSkill skillPacket;
	skillPacket.set_skillid(0);
	skillPacket.set_actorid(this->ActorInfo.objects().actorid());
	skillPacket.set_targetactorid(targetActor->ActorInfo.objects().actorid());
	auto _sendBuffer = WorldPacketHandler::MakeSendBufferPtr(skillPacket);
	this->ownerSession->PostSend(_sendBuffer);

	//update cooltime
	int32 coolTick = 1000;
	nextSkillTimeStamp += coolTick;

	SetMoveState(MoveState::SKILL);
	BroadcastMove(this->ActorInfo.objects().actorid(), Vector2Int::GetVectorFromActorPos(this->ActorInfo.objects().posinfo()));
}

void Monster::UpdateDead()
{

}


void Monster::SetMoveState(MoveState _state) 
{
	ActorInfo.mutable_objects()->mutable_posinfo()->set_state((uint32)_state);
}

void Monster::BroadcastMove(uint32 actorId, Vector2Int pos)
{
	protocol::RequestMove movePacket;
	protocol::PositionInfo posInfo;
	posInfo.set_posx(pos.x);
	posInfo.set_posy(pos.y);

	movePacket.set_actorid(actorId);
	movePacket.mutable_posinfo()->CopyFrom(posInfo);
	auto sendBuffer = WorldPacketHandler::MakeSendBufferPtr(movePacket);
	this->ownerSession->PostSend(sendBuffer);
}