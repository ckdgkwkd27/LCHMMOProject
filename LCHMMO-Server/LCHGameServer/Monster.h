#pragma once
#include "Actor.h"
#include "Zone.h"

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
	virtual void Update();

	void UpdateIdle();
	void UpdateMoving();
	void UpdateChasing();
	void UpdateSkill();
	void UpdateDead();

	void SetMoveState(MoveState _state);
	void BroadcastMove();

private:
	ActorPtr targetActor;
	int64 nextIdleTimeStamp = 0;
	int64 nextMoveTimeStamp = 0;
	int64 nextChaseTimeStamp = 0;
	int64 nextSkillTimeStamp = 0;

	uint32 RoamRadius;
};

using MonsterPtr = std::shared_ptr<Monster>;