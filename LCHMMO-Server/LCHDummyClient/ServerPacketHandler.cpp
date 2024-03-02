#include "pch.h"
#include "ServerPacketHandler.h"
#include "RandomUtil.h"

ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];
std::vector<ServerSession*> GSessionVector;
std::mutex handlerLock;

void ServerPacketHandler::Init()
{
	for (uint32 i = 0; i < UINT16_MAX; i++)
	{
		GServerPacketHandler[i] = HandleInvalid;
	}

	GServerPacketHandler[PKT_SC_LOGIN] = [](ServerSessionPtr& session, char* buffer, uint32 len) 
	{ 
		return HandlePacket<protocol::ReturnLogin>(Handle_PKT_SC_LOGIN, session, buffer, len); 
	};
	GServerPacketHandler[PKT_SC_ENTER_GAME] = [](ServerSessionPtr& session, char* buffer, uint32 len) 
	{ 
		return HandlePacket<protocol::ReturnEnterGame>(Handle_PKT_SC_ENTER_GAME, session, buffer, len);
	};
	GServerPacketHandler[PKT_SC_SPAWN] = [](ServerSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::NotifySpawn>(Handle_PKT_SC_SPAWN, session, buffer, len); 
	};
	GServerPacketHandler[PKT_SC_MOVE] = [](ServerSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::ReturnMove>(Handle_PKT_SC_MOVE, session, buffer, len);
	};
	GServerPacketHandler[PKT_SC_SET_HP] = [](ServerSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::NotifySetHp>(Handle_PKT_SC_SET_HP, session, buffer, len);
	};
	GServerPacketHandler[PKT_SC_SKILL] = [](ServerSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::ReturnSkill>(Handle_PKT_SC_SKILL, session, buffer, len);
	};
	GServerPacketHandler[PKT_SC_DIE] = [](ServerSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::NotifyDie>(Handle_PKT_SC_DIE, session, buffer, len);
	};
	GServerPacketHandler[PKT_SC_DESPAWN] = [](ServerSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::NotifyDespawn>(Handle_PKT_SC_DESPAWN, session, buffer, len);
	};
}

bool ServerPacketHandler::HandlePacket(ServerSessionPtr session, char* buffer, uint32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	return GServerPacketHandler[header->id](session, buffer, len);
}

bool HandleInvalid(ServerSessionPtr& session, char* buffer, uint32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	std::cout << "[FAIL] Invalid Packet. Socket=" << session->GetSocket() << ", ID=" << header->id << ", len = " << len << std::endl;
	return false;
}

bool Handle_PKT_SC_LOGIN(ServerSessionPtr& session, protocol::ReturnLogin& packet)
{
	if (session->state == SessionState::NONE)
		return false;
	
	if (packet.result() == 0)
	{
		std::cout << "[INFO] Login Completed PlayerID: " << packet.playerid() << std::endl;
		session->playerId = packet.playerid();
		session->state = SessionState::LOGIN;

		protocol::RequestEnterGame RequestPkt;
		RequestPkt.set_playerid(packet.playerid());
		RequestPkt.set_zoneid(1);
		auto _sendBuffer = ServerPacketHandler::MakeSendBufferPtr(RequestPkt);
		session->PostSend(_sendBuffer);
		return true;
	}

	else
	{
		std::cout << "[FAILURE] Login Failed!" << std::endl;
		return false;
	}
}

bool Handle_PKT_SC_ENTER_GAME(ServerSessionPtr& session, protocol::ReturnEnterGame& packet)
{
	LockGuard guard(handlerLock);

	int32 x = RandomUtil::GetRandomRangeInt(-506, 399);
	int32 y = RandomUtil::GetRandomRangeInt(-384, 207);
	session->posX = x;
	session->posY = y;

	protocol::RequestMove movePkt;
	movePkt.mutable_posinfo()->set_posx(x);
	movePkt.mutable_posinfo()->set_posy(y);
	auto _sendBuffer = ServerPacketHandler::MakeSendBufferPtr(movePkt);
	session->PostSend(_sendBuffer);

	session->state = SessionState::ENTER_GAME;
	return true;
}

bool Handle_PKT_SC_SPAWN(ServerSessionPtr& session, protocol::NotifySpawn& packet)
{
	std::cout << "Handle_PKT_SC_SPAWN()" << std::endl;
	return true;
}

bool Handle_PKT_SC_MOVE(ServerSessionPtr& session, protocol::ReturnMove& packet)
{
	return true;
}

bool Handle_PKT_SC_SET_HP(ServerSessionPtr& session, protocol::NotifySetHp& packet)
{
	return true;
}

bool Handle_PKT_SC_SKILL(ServerSessionPtr& session, protocol::ReturnSkill& packet)
{
	return true;
}

bool Handle_PKT_SC_DIE(ServerSessionPtr& session, protocol::NotifyDie& packet)
{
	return true;
}

bool Handle_PKT_SC_DESPAWN(ServerSessionPtr& session, protocol::NotifyDespawn& packet)
{
	return true;
}