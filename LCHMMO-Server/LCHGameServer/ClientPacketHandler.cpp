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
	GClientPacketHandler[PKT_CS_INIT_CLIENT] = [](ClientSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::NotifyInitClient>(Handle_PKT_CS_INIT_CLIENT, session, buffer, len);
	};
	GClientPacketHandler[PKT_CS_NPC_SPAWN] = [](ClientSessionPtr& session, char* buffer, uint32 len)
	{
		return HandlePacket<protocol::RequestNpcSpawn>(Handle_PKT_CS_NPC_SPAWN, session, buffer, len);
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

    session->ServiceType = SessionServiceType::USER;
	_zone->messageQueue.Push([_zone, _player]() {_zone->EnterGame(_player, _zone->zoneID); });
    _player->viewport = std::make_shared<Viewport>(_player);
    _player->zoneID = _zone->zoneID;

	return true;
}

bool Handle_PKT_CS_MOVE(ClientSessionPtr& session, protocol::RequestMove& packet)
{
    if(session->ServiceType == SessionServiceType::USER)
	{
		PlayerPtr _player = session->currentPlayer;
		RETURN_FALSE_ON_FAIL(_player != nullptr);

		ZonePtr _zone = GZoneManager.FindZoneByID(_player->zoneID);
		RETURN_FALSE_ON_FAIL(_zone != nullptr);

		_zone->messageQueue.Push([_zone, _player, packet]() {_zone->HandleMove(_player, packet); });
	}

    else if (session->ServiceType == SessionServiceType::NPC)
    {
        uint32 actorId = packet.actorid();
        ZonePtr _zone = GZoneManager.FindZoneByID(1);
        ActorPtr _actor = _zone->FindActor(static_cast<ActorIDType>(actorId));
        if (_actor == nullptr)
            return false;

        _actor->ActorInfo.mutable_posinfo()->set_state((uint32)MoveState::MOVING);
        _zone->messageQueue.Push([_zone, _actor, packet]() {_zone->HandleMove(_actor, packet); });

        _zone->messageQueue.Push([_zone, _actor, packet]() 
            {
                protocol::ReturnMove movePacket;
                movePacket.set_actorid(_actor->ActorInfo.actorid());
                movePacket.mutable_posinfo()->CopyFrom(_actor->ActorInfo.posinfo());
                auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(movePacket);
                _zone->BroadCast(_actor, _sendBuffer);

                SectionPtr section = _zone->GetSection(Vector2Int::GetVectorFromActorPos(packet.posinfo()));
                section->PlayerViewportUpdate();
            });
    }
    return true;
}

bool Handle_PKT_CS_SKILL(ClientSessionPtr& session, protocol::RequestSkill& packet)
{
    if (auto const _player = session->currentPlayer; _player != nullptr)
    {
        RETURN_FALSE_ON_FAIL(_player != nullptr && _player->ActorInfo.statinfo().hp() > 0);

        ZonePtr _zone = GZoneManager.FindZoneByID(_player->zoneID);
        RETURN_FALSE_ON_FAIL(_zone != nullptr);

        _zone->messageQueue.Push([_zone, _player, packet] {_zone->HandleSkill(_player, packet); });
    }

    //Npc가 쓰는 스킬
    else
    {
        uint32 actorId = packet.actorid();
        auto const _zone = GZoneManager.FindZoneByID(1);
        RETURN_FALSE_ON_FAIL(_zone != nullptr);

        auto const monster_ = _zone->FindActor(static_cast<ActorIDType>(actorId));
        RETURN_FALSE_ON_FAIL(monster_ != nullptr);

        _zone->messageQueue.Push([_zone, monster_, packet] {_zone->HandleSkill(monster_, packet); });
    }
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

    //Player Despawn 처리, 신규 Zone 입장처리
    //currentZone->UnregisterActor(_player);

    _player->zoneID = packet.zoneid();
    _player->ActorInfo.mutable_posinfo()->set_posx(packet.posinfo().posx());
    _player->ActorInfo.mutable_posinfo()->set_posy(packet.posinfo().posy());

	//Despawn Old
    currentZone->LeaveGame(_player->ActorInfo.actorid());

    //Notify
    newZone->messageQueue.Push([newZone, _player]() {newZone->EnterGame(_player, newZone->zoneID); });

	//Npc Session에게만 발송
	if (_player->zoneID == 1)
	{
        newZone->messageQueue.Push([_player, newZone]
			{
				for (auto _session : GSessionManager.activeSessions)
				{
					ClientSessionPtr clientSession = std::static_pointer_cast<ClientSession>(_session);
					if (clientSession->ServiceType == SessionServiceType::NPC)
					{
						protocol::ReturnEnterGame enterPacket;
						enterPacket.mutable_myplayer()->CopyFrom(_player->ActorInfo);
						enterPacket.set_zoneid((uint32)newZone->zoneID);
						auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(enterPacket);
						_session->PostSend(_sendBuffer);
					}
				}
			}
		);
	}

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

bool Handle_PKT_CS_INIT_CLIENT(ClientSessionPtr& session, protocol::NotifyInitClient& packet)
{
    if (packet.serviceid() != static_cast<uint32>(SessionServiceType::NPC))
        printf_s("[FAIL] Invalid ServiceId=%d, Socket=%lld\n", packet.serviceid(), session->GetSocket());

    session->ServiceType = SessionServiceType::NPC;
    std::cout << "[INFO] Init NPC Client SUCCESS" << std::endl;
    return true;
}

bool Handle_PKT_CS_NPC_SPAWN(ClientSessionPtr& session, protocol::RequestNpcSpawn& packet)
{
    for (uint32 idx = 0; idx < static_cast<uint32>(packet.npcbuf_size()); idx++)
    {
		MonsterPtr monster = std::make_shared<Monster>();
		monster->zoneID = packet.zoneid();
        monster->ActorInfo = packet.npcbuf(idx).objects();
        monster->ActorInfo.set_actorid(GZoneManager.IssueActorID());
        packet.mutable_npcbuf(idx)->mutable_objects()->set_actorid(monster->ActorInfo.actorid());
        
        ZonePtr _zone = GZoneManager.FindZoneByID(monster->zoneID);
        RETURN_FALSE_ON_FAIL(_zone != nullptr);
        _zone->messageQueue.Push([_zone, monster]() {_zone->RegisterActor(monster); });
    }

    protocol::ReturnNpcSpawn spawnPkt;
    spawnPkt.mutable_npcbuf()->CopyFrom(packet.npcbuf());
    auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(spawnPkt);
    RETURN_FALSE_ON_FAIL(session->PostSend(_sendBuffer));

    std::cout << "[INFO] NPC SPAWN SUCCESS" << std::endl;
    return true;
}
