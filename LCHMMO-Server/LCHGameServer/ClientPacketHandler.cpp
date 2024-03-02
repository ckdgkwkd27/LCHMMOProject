#include "pch.h"
#include "ClientPacketHandler.h"
#include "PlayerManager.h"
#include "ZoneManager.h"
#include "TimeUtil.h"
#include "Viewport.h"
#include "PlayerDBMessage.h"
#include "DBManager.h"

ClientPacketHandlerFunc GClientPacketHandler[UINT16_MAX];

void ClientPacketHandler::Init()
{
    for (uint32 i = 0; i < UINT16_MAX; i++)
    {
        GClientPacketHandler[i] = HandleInvalid;
    }

	GClientPacketHandler[PKT_CS_JOIN] = [](ClientSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::RequestJoin>(Handle_PKT_CS_JOIN, session, buffer, len);
	};
    GClientPacketHandler[PKT_CS_LOGIN] = [](ClientSessionPtr& session, char* buffer, uint32 len) 
    { 
        return HandlePacket<protocol::RequestLogin>(Handle_PKT_CS_LOGIN, session, buffer, len); 
    };
    GClientPacketHandler[PKT_CS_ENTER_GAME] = [](ClientSessionPtr& session, char* buffer, uint32 len)
    {
        return HandlePacket<protocol::RequestEnterGame>(Handle_PKT_CS_ENTER_GAME, session, buffer, len);
    };
    GClientPacketHandler[PKT_CS_MOVE] = [](ClientSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::RequestMove>(Handle_PKT_CS_MOVE, session, buffer, len);
	};
	GClientPacketHandler[PKT_CS_SKILL] = [](ClientSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::RequestSkill>(Handle_PKT_CS_SKILL, session, buffer, len);
	};
	GClientPacketHandler[PKT_CS_TELEPORT] = [](ClientSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::RequestTeleport>(Handle_PKT_CS_TELEPORT, session, buffer, len);
	};
	GClientPacketHandler[PKT_S_VIEWPORT_UDPATE] = [](ClientSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::RequestViewportUpdate>(Handle_PKT_S_VIEWPORT_UPDATE, session, buffer, len);
	};
}

bool ClientPacketHandler::HandlePacket(ClientSessionPtr session, char* buffer, uint32 len)
{
    PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
    return GClientPacketHandler[header->id](session, buffer, len);
}

bool HandleInvalid(ClientSessionPtr& session, char* buffer, uint32 len)
{
    PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
    std::cout << "[FAIL] Invalid Packet. Socket=" << session->GetSocket() << ", ID=" << header->id << ", len = " << len << std::endl;
    return false;
}

bool Handle_PKT_CS_JOIN(ClientSessionPtr& session, protocol::RequestJoin& packet)
{
    JoinPlayerDBMessage* message = new JoinPlayerDBMessage(session);
    if (packet.id().length() > 20 || packet.password().length() > 20)
        return false;

    message->id = packet.id();
    message->pwd = packet.password();
    message->playerSession = session;
    GDBManager->PostDBMessage(message);
    return true;
}

bool Handle_PKT_CS_LOGIN(ClientSessionPtr& session, protocol::RequestLogin& packet)
{
    LoginPlayerDBMessage* message = new LoginPlayerDBMessage(session);
	message->id = packet.id();
	message->pwd = packet.password();
	message->playerSession = session;
	GDBManager->PostDBMessage(message);
    return true;
}

bool Handle_PKT_CS_ENTER_GAME(ClientSessionPtr& session, protocol::RequestEnterGame& packet)
{
    if (session->currentPlayer->playerId != packet.playerid())
        return false;

	PlayerPtr _player = session->currentPlayer;
	RETURN_FALSE_ON_FAIL(_player != nullptr);

	ZonePtr _zone = GZoneManager.FindZoneByID(packet.zoneid());
	RETURN_FALSE_ON_FAIL(_zone != nullptr);

	_zone->messageQueue.Push([_zone, _player]() {_zone->EnterGame(_player, _zone->zoneID); });
    _player->viewport = std::make_shared<Viewport>(_player);
    _player->zoneID = _zone->zoneID;

    if (packet.playerid() > 1300)
	    std::cout << "[INFO] ReturnEnterGame Packet Send Socket=" << session->GetSocket() << ", PlayerID=" << packet.playerid() << ", ZoneID=" << _zone->zoneID << std::endl;
	return true;
}

bool Handle_PKT_CS_MOVE(ClientSessionPtr& session, protocol::RequestMove& packet)
{
    PlayerPtr _player = session->currentPlayer;
    RETURN_FALSE_ON_FAIL(_player != nullptr);

    ZonePtr _zone = GZoneManager.FindZoneByID(_player->zoneID);
    RETURN_FALSE_ON_FAIL(_zone != nullptr);

    _zone->messageQueue.Push([_zone, _player, packet]() {_zone->HandleMove(_player, packet); });
    return true;
}

bool Handle_PKT_CS_SKILL(ClientSessionPtr& session, protocol::RequestSkill& packet)
{
    PlayerPtr _player = session->currentPlayer;
    RETURN_FALSE_ON_FAIL(_player != nullptr);

    ZonePtr _zone = GZoneManager.FindZoneByID(_player->zoneID);
    RETURN_FALSE_ON_FAIL(_zone != nullptr);

    _zone->messageQueue.Push([_zone, _player, packet] {_zone->HandleSkill(_player, packet); });
    return true;
}

bool Handle_PKT_CS_TELEPORT(ClientSessionPtr& session, protocol::RequestTeleport& packet)
{
    PlayerPtr _player = session->currentPlayer;
    RETURN_FALSE_ON_FAIL(_player != nullptr);

    ZonePtr currentZone = GZoneManager.FindZoneByID(_player->zoneID);
    RETURN_FALSE_ON_FAIL(currentZone != nullptr);

    ZonePtr newZone = GZoneManager.FindZoneByID(packet.zoneid());
    RETURN_FALSE_ON_FAIL(newZone != nullptr);

    //Player Despawn 贸府, 脚痹 Zone 涝厘贸府
    //currentZone->UnregisterActor(_player);

    _player->zoneID = packet.zoneid();
    _player->ActorInfo.mutable_posinfo()->set_posx(packet.posinfo().posx());
    _player->ActorInfo.mutable_posinfo()->set_posy(packet.posinfo().posy());

	//Despawn Old
    currentZone->LeaveGame(_player->ActorInfo.actorid());

    //Notify
    newZone->messageQueue.Push([newZone, _player]() {newZone->EnterGame(_player, newZone->zoneID); });

    std::cout << "[INFO] Handle TELEPORT ActorId=" << packet.actorid() << ", ZoneId=" << packet.zoneid() << std::endl;
    return true;
}

bool Handle_PKT_S_VIEWPORT_UPDATE(ClientSessionPtr& session, protocol::RequestViewportUpdate& packet)
{
	PlayerPtr _player = session->currentPlayer;
	RETURN_FALSE_ON_FAIL(_player != nullptr);
    RETURN_FALSE_ON_FAIL(_player->playerId == packet.playerid());
    
	ZonePtr _zone = GZoneManager.FindZoneByID(_player->zoneID);
	RETURN_FALSE_ON_FAIL(_zone != nullptr);

	_zone->messageQueue.Push([_player] {_player->viewport->Update(); });
    return true;
}
