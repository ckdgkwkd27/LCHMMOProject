#include "pch.h"
#include "WorldSession.h"
#include "WorldPacketHandler.h"
#include "NpcActor.h"

uint32 WorldSession::GlobalNpcIDCnt = 0;

void WorldSession::OnAccepted()
{
}

void WorldSession::OnConnected()
{
	std::cout << "[INFO] Npc Server Connected.." << std::endl;

	protocol::NotifyInitClient InitPacket;
	InitPacket.set_serviceid(serviceId);
	auto _sendBuffer = WorldPacketHandler::MakeSendBufferPtr(InitPacket);
	PostSend(_sendBuffer);

	protocol::RequestNpcSpawn spawnPacket;
	for(uint32 idx = 0; idx < 500; idx++)
	{
		spawnPacket.set_zoneid(1);
		protocol::NpcInfo* info = spawnPacket.add_npcbuf();
		info->set_npcid(GlobalNpcIDCnt);
		info->mutable_objects()->set_objecttype((uint32)ObjectType::MONSTER);
		info->mutable_objects()->set_name("Monster");
		info->mutable_objects()->mutable_posinfo()->set_state((uint32)MoveState::IDLE);
		info->mutable_objects()->mutable_statinfo()->set_level(1);
		info->mutable_objects()->mutable_statinfo()->set_hp(10);
		info->mutable_objects()->mutable_statinfo()->set_maxhp(10);
		info->mutable_objects()->mutable_statinfo()->set_attack(5);
		info->mutable_objects()->mutable_statinfo()->set_speed(2);
		info->mutable_objects()->mutable_statinfo()->set_totalexp(0);
		++GlobalNpcIDCnt;
	}

	auto _spawnBuffer = WorldPacketHandler::MakeSendBufferPtr(spawnPacket);
	PostSend(_spawnBuffer);
}

void WorldSession::OnDisconnected()
{
	
}

uint32 WorldSession::OnRecv(char* buffer, uint32 len)
{
	uint32 processLen = 0;

	while (true)
	{
		uint32 dataSize = len - processLen;
		if (dataSize < sizeof(PacketHeader))
			break;

		PacketHeader header = *(reinterpret_cast<PacketHeader*>(&buffer[processLen]));
		if (dataSize < header.size)
			break;

		WorldPacketHandler::HandlePacket(std::dynamic_pointer_cast<WorldSession>(shared_from_this()), buffer, len);
		processLen += header.size;
	}

	return processLen;
}

