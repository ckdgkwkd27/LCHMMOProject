#include "pch.h"
#include "Actor.h"
#include "Map.h"
#include "ZoneManager.h"
#include "ClientPacketHandler.h"

MoveDir Actor::GetDirFromVector(Vector2Int vec)
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

void Actor::OnDamaged(std::shared_ptr<Actor> attacker, int32 damage)
{
	auto zone = GZoneManager.FindZoneByID(zoneID);
	if (zone == nullptr)
		return;

	int32 actorHp = ActorInfo.statinfo().hp();
	ActorInfo.mutable_statinfo()->set_hp(std::max(actorHp - damage, 0));

	protocol::NotifySetHp hpPacket;
	hpPacket.set_actorid(attacker->ActorInfo.actorid());
	hpPacket.set_hp(ActorInfo.statinfo().hp());
	auto sendBufer = ClientPacketHandler::MakeSendBufferPtr(hpPacket);
	zone->BroadCast(Vector2Int::GetVectorFromActorPos(ActorInfo.posinfo()), sendBufer);

	if (ActorInfo.statinfo().hp() <= 0)
	{
		OnDead(attacker);
	}
}

void Actor::OnDead(std::shared_ptr<Actor> attacker)
{
	auto zone = GZoneManager.FindZoneByID(zoneID);
	if (zone == nullptr)
		return;

	protocol::NotifyDie diePacket;
	diePacket.set_actorid(ActorInfo.actorid());
	diePacket.set_attackerid(attacker->ActorInfo.actorid());
	auto sendBufer = ClientPacketHandler::MakeSendBufferPtr(diePacket);
	zone->BroadCast(Vector2Int::GetVectorFromActorPos(ActorInfo.posinfo()), sendBufer);

	zone->LeaveGame(ActorInfo.actorid());
	ActorInfo.mutable_statinfo()->set_hp(ActorInfo.statinfo().maxhp());
	ActorInfo.mutable_posinfo()->set_state((uint32)MoveState::DEAD);
	ActorInfo.mutable_posinfo()->set_movedir((uint32)MoveDir::DOWN);
}
