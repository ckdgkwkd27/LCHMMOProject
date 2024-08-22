#pragma once
#include "ClientSession.h"
#include "protocol.pb.h"

using ClientSessionPtr = std::shared_ptr<ClientSession>;
using ClientPacketHandlerFunc = std::function<bool(ClientSessionPtr&, char*, uint32)>;

extern ClientPacketHandlerFunc GClientPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_CS_JOIN = 1,
	PKT_SC_JOIN,
	PKT_CS_LOGIN ,
	PKT_SC_LOGIN,
	PKT_CS_ENTER_GAME,
	PKT_SC_ENTER_GAME,
	PKT_SC_SPAWN,
	PKT_CS_MOVE,
	PKT_SC_MOVE,
	PKT_SC_SET_HP,
	PKT_CS_SKILL,
	PKT_SC_SKILL,
	PKT_SC_DIE,
	PKT_SC_DESPAWN,
	PKT_CS_TELEPORT,
	PKT_SC_TELEPORT,
	PKT_S_VIEWPORT_UDPATE,
	PKT_CS_INIT_CLIENT,
	PKT_CS_NPC_SPAWN,
	PKT_SC_NPC_SPAWN,
	PKT_SC_WORLD_TICK
};

bool HandleInvalid(ClientSessionPtr& session, char* buffer, uint32 len);
bool Handle_PKT_CS_JOIN(ClientSessionPtr& session, protocol::RequestJoin& packet);
bool Handle_PKT_CS_LOGIN(ClientSessionPtr& session, protocol::RequestLogin& packet);
bool Handle_PKT_CS_ENTER_GAME(ClientSessionPtr& session, protocol::RequestEnterGame& packet);
bool Handle_PKT_CS_MOVE(ClientSessionPtr& session, protocol::RequestMove& packet);
bool Handle_PKT_CS_SKILL(ClientSessionPtr& session, protocol::RequestSkill& packet);
bool Handle_PKT_CS_TELEPORT(ClientSessionPtr& session, protocol::RequestTeleport& packet);
bool Handle_PKT_S_VIEWPORT_UPDATE(ClientSessionPtr& session, protocol::RequestViewportUpdate& packet);
bool Handle_PKT_CS_INIT_CLIENT(ClientSessionPtr& session, protocol::NotifyInitClient& packet);
bool Handle_PKT_CS_NPC_SPAWN(ClientSessionPtr& session, protocol::RequestNpcSpawn& packet);

class ClientPacketHandler
{
public:
	static void Init();
	static bool HandlePacket(ClientSessionPtr session, char* buffer, uint32 len);
	static CircularBufferPtr MakeSendBufferPtr(protocol::ReturnJoin& pkt) { return MakeSendBufferPtr(pkt, PKT_SC_JOIN); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::ReturnLogin& pkt) { return MakeSendBufferPtr(pkt, PKT_SC_LOGIN); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::ReturnEnterGame& pkt) { return MakeSendBufferPtr(pkt, PKT_SC_ENTER_GAME); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::NotifySpawn& pkt) { return MakeSendBufferPtr(pkt, PKT_SC_SPAWN); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::ReturnMove& pkt) { return MakeSendBufferPtr(pkt, PKT_SC_MOVE); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::NotifySetHp& pkt) { return MakeSendBufferPtr(pkt, PKT_SC_SET_HP); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::ReturnSkill& pkt) { return MakeSendBufferPtr(pkt, PKT_SC_SKILL); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::NotifyDie& pkt) { return MakeSendBufferPtr(pkt, PKT_SC_DIE); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::NotifyDespawn& pkt) { return MakeSendBufferPtr(pkt, PKT_SC_DESPAWN); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::RequestViewportUpdate& pkt) { return MakeSendBufferPtr(pkt, PKT_S_VIEWPORT_UDPATE); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::ReturnNpcSpawn& pkt) { return MakeSendBufferPtr(pkt, PKT_SC_NPC_SPAWN); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::NotifyWorldTick& pkt) { return MakeSendBufferPtr(pkt, PKT_SC_WORLD_TICK); }

	template<typename T>
	static CircularBufferPtr MakeSendBufferPtr(T& pkt, uint16 PacketID)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		CircularBufferPtr sendBuffer = std::make_shared<CircularBuffer>(MAX_BUFFER_SIZE);
		if (sendBuffer == nullptr)
			CRASH_ASSERT(false);

		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->data());
		header->size = packetSize;
		header->id = PacketID;
		CRASH_ASSERT(pkt.SerializeToArray(&header[1], dataSize));
		CRASH_ASSERT(sendBuffer->OnWrite(packetSize) != false);
		return sendBuffer;
	}

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, ClientSessionPtr& session, char* buffer, uint32 len);
};

template<typename PacketType, typename ProcessFunc>
inline bool ClientPacketHandler::HandlePacket(ProcessFunc func, ClientSessionPtr& session, char* buffer, uint32 len)
{
	PacketType packet;
	if (packet.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
		return false;

	return func(session, packet);
}
