#pragma once
#include "protocol.pb.h"
enum class ObjectType : uint32
{
	NONE = 0,
	PLAYER,
	MONSTER,
	PROJECTILE
};

enum class MoveState : uint32
{
	IDLE = 0,
	MOVING,
	CHASING,
	SKILL,
	DEAD,
};

enum class MoveDir : uint32
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};

class Actor : public std::enable_shared_from_this<Actor>
{
public:
	virtual ~Actor() {}
	virtual void Update() {}
	virtual void OnDamaged(std::shared_ptr<Actor> attacker, int32 damage) {}
	virtual void OnDead(std::shared_ptr<Actor> attacker) {}

public:
	protocol::NpcInfo ActorInfo;
	uint32 ZoneID;
};

using ActorPtr = std::shared_ptr<Actor>;