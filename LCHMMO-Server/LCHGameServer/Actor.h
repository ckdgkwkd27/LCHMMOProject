#pragma once
#include "TimeUtil.h"
#include "protocol.pb.h"
using ActorIDType = uint32;


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

class Vector2Int;
class ZoneManager;
class Actor : public std::enable_shared_from_this<Actor>
{
public:
	virtual ~Actor() {}
	virtual void Update() {}
	virtual void OnDamaged(std::shared_ptr<Actor> attacker, int32 damage);
	virtual void OnDead(std::shared_ptr<Actor> attacker);
	virtual void ViewportUpdate() {}
	MoveDir GetDirFromVector(Vector2Int vec);


public:
	protocol::ObjectInfo ActorInfo;
	uint32 zoneID;
	TimeStampType LastViewportUpdateTimeStamp;
};

using ActorPtr = std::shared_ptr<Actor>;
