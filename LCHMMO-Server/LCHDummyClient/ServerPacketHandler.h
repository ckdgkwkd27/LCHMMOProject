#pragma once
#include "ServerSession.h"
#include "protocol.pb.h"

using ServerSessionPtr = std::shared_ptr<ServerSession>;
using ServerPacketHandlerFunc = std::function<bool(ServerSessionPtr&, char*, uint32)>;

extern ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];
extern std::vector<ServerSession*> GSessionVector;

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
};

bool HandleInvalid(ServerSessionPtr& session, char* buffer, uint32 len);
bool Handle_PKT_SC_LOGIN(ServerSessionPtr& session, protocol::ReturnLogin& packet);
bool Handle_PKT_SC_ENTER_GAME(ServerSessionPtr& session, protocol::ReturnEnterGame& packet);
bool Handle_PKT_SC_SPAWN(ServerSessionPtr& session, protocol::NotifySpawn& packet);
bool Handle_PKT_SC_MOVE(ServerSessionPtr& session, protocol::ReturnMove& packet);
bool Handle_PKT_SC_SET_HP(ServerSessionPtr& session, protocol::NotifySetHp& packet);
bool Handle_PKT_SC_SKILL(ServerSessionPtr& session, protocol::ReturnSkill& packet);
bool Handle_PKT_SC_DIE(ServerSessionPtr& session, protocol::NotifyDie& packet);
bool Handle_PKT_SC_DESPAWN(ServerSessionPtr& session, protocol::NotifyDespawn& packet);
extern std::mutex handlerLock;


class ServerPacketHandler
{
public:
	static void Init();
	static bool HandlePacket(ServerSessionPtr session, char* buffer, uint32 len);
	static CircularBufferPtr MakeSendBufferPtr(protocol::RequestLogin& pkt) { return MakeSendBufferPtr(pkt, PKT_CS_LOGIN); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::RequestEnterGame& pkt) { return MakeSendBufferPtr(pkt, PKT_CS_ENTER_GAME); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::RequestMove& pkt) { return MakeSendBufferPtr(pkt, PKT_CS_MOVE); }
	static CircularBufferPtr MakeSendBufferPtr(protocol::RequestTeleport& pkt) { return MakeSendBufferPtr(pkt, PKT_CS_TELEPORT); }

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
	static bool HandlePacket(ProcessFunc func, ServerSessionPtr& session, char* buffer, uint32 len);
};

template<typename PacketType, typename ProcessFunc>
inline bool ServerPacketHandler::HandlePacket(ProcessFunc func, ServerSessionPtr& session, char* buffer, uint32 len)
{
	PacketType packet;
	if (packet.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
		return false;

	return func(session, packet);
}