#pragma once
#include "NpcActor.h"
#include "Map.h"
#include "WorldSession.h"

#define STATE_CHANGE_INTERVAL 10
#define SEARCH_INTERVAL 10
#define TICK_INTERVAL_IDLE	100
#define TICK_INTERVAL_SKILL 100	

#define SEARCH_CELL_DISTANCE 5
#define CHASE_CELL_DISTANCE 10
#define SKILL_DISTANCE 1

class Monster : public Actor
{
public:
	Monster();
	void Update();

	void UpdateIdle();
	void UpdateMoving();
	void UpdateChasing();
	void UpdateSkill();
	void UpdateDead();

	void SetMoveState(MoveState _state);
	void BroadcastMove(uint32 actorId, Vector2Int pos);

	MoveDir GetDirFromVector(Vector2Int vec)
	{
		if (vec.x > 0)
			return MoveDir::RIGHT;
		else if (vec.x < 0)
			return MoveDir::LEFT;
		else if (vec.y > 0)
			return MoveDir::UP;
		else
			return MoveDir::DOWN;
	}

private:
	ActorPtr targetActor;
	int64 nextIdleTimeStamp = 0;
	int64 nextMoveTimeStamp = 0;
	int64 nextChaseTimeStamp = 0;
	int64 nextSkillTimeStamp = 0;
	uint32 RoamRadius;

public:
	WorldSessionPtr ownerSession;
};

using MonsterPtr = std::shared_ptr<Monster>;