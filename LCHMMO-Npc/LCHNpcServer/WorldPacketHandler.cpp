#include "pch.h"
#include "WorldPacketHandler.h"
#include "Monster.h"
#include "MapManager.h"

WorldPacketHandlerFunc GWorldPacketHandler[UINT16_MAX];

void WorldPacketHandler::Init()
{
	for (uint32 i = 0; i < UINT16_MAX; i++)
	{
		GWorldPacketHandler[i] = HandleInvalid;
	}

	GWorldPacketHandler[PKT_SC_ENTER_GAME] = [](WorldSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::ReturnEnterGame>(Handle_PKT_SC_ENTER_GAME, session, buffer, len);
	};

	GWorldPacketHandler[PKT_SC_SPAWN] = [](WorldSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::NotifySpawn>(Handle_PKT_SC_SPAWN, session, buffer, len);
	};

	GWorldPacketHandler[PKT_SC_MOVE] = [](WorldSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::ReturnMove>(Handle_PKT_SC_MOVE, session, buffer, len);
	};

	GWorldPacketHandler[PKT_SC_NPC_SPAWN] = [](WorldSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::ReturnNpcSpawn>(Handle_PKT_SC_NPC_SPAWN, session, buffer, len);
	};

	GWorldPacketHandler[PKT_SC_WORLD_TICK] = [](WorldSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::NotifyWorldTick>(Handle_PKT_SC_WORLD_TICK, session, buffer, len);
	};
}

bool WorldPacketHandler::HandlePacket(WorldSessionPtr session, char* buffer, uint32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	return GWorldPacketHandler[header->id](session, buffer, len);
}

bool HandleInvalid(WorldSessionPtr& session, char* buffer, uint32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	std::cout << "[FAIL] Invalid Packet. Socket=" << session->GetSocket() << ", ID=" << header->id << ", len = " << len << std::endl;
	return false;
}

bool Handle_PKT_SC_ENTER_GAME(WorldSessionPtr& session, protocol::ReturnEnterGame& packet)
{
	uint32 actorId = packet.myplayer().actorid();
	if (GActorManager.ActorIDToActorHashMap.find(actorId) != GActorManager.ActorIDToActorHashMap.end())
	{
		return false;
	}

	ActorPtr player = MACRO_NEW(Actor);
	player->ActorInfo.mutable_objects()->CopyFrom(packet.myplayer());
	GActorManager.ActorIDToActorHashMap.insert({ actorId, player });
	return true;
}

bool Handle_PKT_SC_SPAWN(WorldSessionPtr& session, protocol::NotifySpawn& packet)
{
	for (uint32 idx = 0; idx < static_cast<uint32>(packet.objects_size()); idx++)
	{
		ActorPtr pActor = MACRO_NEW(Actor);
		pActor->ActorInfo.CopyFrom(packet.objects(idx));

		uint32 ActorID = packet.objects(idx).actorid();
		if (GActorManager.ActorIDToActorHashMap.insert({ ActorID, pActor }).second == false)
		{
			std::cout << "Player Duplicated ERROR!" << std::endl;
			return false;
		}
	}
	return true;
}

bool Handle_PKT_SC_MOVE(WorldSessionPtr& session, protocol::ReturnMove& packet)
{
	uint32 actorId = packet.actorid();
	auto it = GActorManager.ActorIDToActorHashMap.find(actorId);
	if (it == GActorManager.ActorIDToActorHashMap.end())
	{
		return false;
	}

	auto const _map = GMapManager.FindMap(1);
	Vector2Int v(packet.posinfo().posx(), packet.posinfo().posy());
	_map->ApplyMove(it->second, v);
	it->second->ActorInfo.mutable_objects()->mutable_posinfo()->CopyFrom(packet.posinfo());
	return true;
}

bool Handle_PKT_SC_NPC_SPAWN(WorldSessionPtr& session, protocol::ReturnNpcSpawn& packet)
{
	for(uint32 idx = 0; idx < static_cast<uint32>(packet.npcbuf_size()); idx++)
	{
		MonsterPtr pActor = MACRO_NEW(Monster);
		pActor->ActorInfo.CopyFrom(packet.npcbuf(idx));
		pActor->ZoneID = 1;
		pActor->ownerSession = session;

		uint32 ActorID = packet.npcbuf(idx).objects().actorid();
		if (GActorManager.ActorIDToActorHashMap.insert({ ActorID, pActor }).second == false)
		{
			std::cout << "Npc Duplicated ERROR!" << std::endl;
			return false;
		}
	}

	std::cout << "Npc Creation SUCCESS(SessionID=" << session->GetSocket() << ")" << std::endl;
	return true;
}

bool Handle_PKT_SC_WORLD_TICK(WorldSessionPtr& session, protocol::NotifyWorldTick& packet)
{
	for (auto actorPair : GActorManager.ActorIDToActorHashMap)
	{
		ActorPtr actor = actorPair.second;
		actor->Update();
	}
	return true;
}
