#pragma once
#include "WorldSession.h"
#include "protocol.pb.h"
#include "ActorManager.h"

using WorldPacketHandlerFunc = std::function<bool(WorldSessionPtr&, char*, uint32)>;
extern WorldPacketHandlerFunc GWorldPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_CS_JOIN = 1,
	PKT_SC_JOIN,
	PKT_CS_LOGIN,
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
	PKT_SC_WORLD_TICK,
};

bool HandleInvalid(WorldSessionPtr& session, char* buffer, uint32 len);
bool Handle_PKT_SC_ENTER_GAME(WorldSessionPtr& session, protocol::ReturnEnterGame& packet);
bool Handle_PKT_SC_SPAWN(WorldSessionPtr& session, protocol::NotifySpawn& packet);
bool Handle_PKT_SC_MOVE(WorldSessionPtr& session, protocol::ReturnMove& packet);
bool Handle_PKT_SC_NPC_SPAWN(WorldSessionPtr& session, protocol::ReturnNpcSpawn& packet);
bool Handle_PKT_SC_WORLD_TICK(WorldSessionPtr& session, protocol::NotifyWorldTick& packet);

class WorldPacketHandler
{
public:
	static void Init();
	static bool HandlePacket(WorldSessionPtr session, char* buffer, uint32 len);
	static CircularBufferPtr MakeSendBufferPtr(protocol::NotifyInitClient& pkt) { return MakeSendBufferPtr(pkt, PKT_CS_INIT_CLIENT); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::RequestNpcSpawn& pkt) { return MakeSendBufferPtr(pkt, PKT_CS_NPC_SPAWN); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::RequestMove& pkt) { return MakeSendBufferPtr(pkt, PKT_CS_MOVE); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::RequestSkill& pkt) { return MakeSendBufferPtr(pkt, PKT_CS_SKILL); }

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
	static bool HandlePacket(ProcessFunc func, WorldSessionPtr& session, char* buffer, uint32 len);
};

template<typename PacketType, typename ProcessFunc>
inline bool WorldPacketHandler::HandlePacket(ProcessFunc func, WorldSessionPtr& session, char* buffer, uint32 len)
{
	PacketType packet;
	if (packet.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
		return false;

	return func(session, packet);
}