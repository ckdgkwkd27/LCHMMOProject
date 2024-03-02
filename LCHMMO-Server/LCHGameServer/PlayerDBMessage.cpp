#include "pch.h"
#include "PlayerDBMessage.h"
#include "SqlStatement.h"
#include "protocol.pb.h"
#include "ClientPacketHandler.h"
#include "PlayerManager.h"
#include "ZoneManager.h"

JoinPlayerDBMessage::JoinPlayerDBMessage(ClientSessionPtr owner)
	: DBMessage(owner)
{
	
}

void JoinPlayerDBMessage::OnSuccess()
{
	protocol::ReturnJoin joinPkt;
	joinPkt.set_result(resultId);
	auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(joinPkt);
	playerSession->PostSend(_sendBuffer);
}

void JoinPlayerDBMessage::OnFail()
{
	protocol::ReturnJoin joinPkt;
	joinPkt.set_result(resultId);
	auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(joinPkt);
	playerSession->PostSend(_sendBuffer);
}

bool JoinPlayerDBMessage::OnSqlExecute()
{
	DBHelper dbHelper;

	dbHelper.BindParamTextA(id.c_str());
	dbHelper.BindParamTextA(pwd.c_str());

	dbHelper.BindResultColumnInt(&resultId);

	if (dbHelper.Execute(SQL_PLAYER_JOIN))
	{
		if(dbHelper.FetchRow())
		{
			printf_s("[INFO] Player Created!\n");
			return resultId == 0;
		}
	}

	return false;
}

LoginPlayerDBMessage::LoginPlayerDBMessage(ClientSessionPtr owner)
	: DBMessage(owner)
{
}

void LoginPlayerDBMessage::OnSuccess()
{
	auto _player = GPlayerManager.NewPlayer();
	_player->ActorInfo.set_objecttype((uint32)ObjectType::PLAYER);
	_player->ActorInfo.set_actorid(GZoneManager.IssueActorID());
	_player->ActorInfo.set_name("Player" + std::to_string(_player->playerId));
	_player->ActorInfo.mutable_posinfo()->set_state((uint32)MoveState::IDLE);
	_player->ActorInfo.mutable_posinfo()->set_movedir((uint32)MoveDir::UP);
	_player->ActorInfo.mutable_posinfo()->set_posx(0);
	_player->ActorInfo.mutable_posinfo()->set_posy(0);
	_player->ActorInfo.mutable_statinfo()->set_level(1);
	_player->ActorInfo.mutable_statinfo()->set_hp(100);
	_player->ActorInfo.mutable_statinfo()->set_maxhp(100);
	_player->ActorInfo.mutable_statinfo()->set_attack(5);
	_player->ActorInfo.mutable_statinfo()->set_speed(5);
	_player->ActorInfo.mutable_statinfo()->set_totalexp(0);
	_player->zoneID = 0;
	_player->ownerSession = playerSession;
	_player->ownerSession->currentPlayer = _player;

	protocol::ReturnLogin loginPkt;
	loginPkt.set_result(resultId);
	loginPkt.set_playerid(_player->playerId);
	auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(loginPkt);
	playerSession->PostSend(_sendBuffer);

	std::cout << "[INFO] Handle Login! Socket=" << _player->ownerSession->GetSocket() << ", ID=" << id << ", Password=" << pwd << std::endl;
}

void LoginPlayerDBMessage::OnFail()
{
	protocol::ReturnLogin loginPkt;
	loginPkt.set_result(resultId);
	loginPkt.set_playerid(0);
	auto _sendBuffer = ClientPacketHandler::MakeSendBufferPtr(loginPkt);
	playerSession->PostSend(_sendBuffer);
}

bool LoginPlayerDBMessage::OnSqlExecute()
{
	DBHelper dbHelper;

	dbHelper.BindParamTextA(id.c_str());
	dbHelper.BindParamTextA(pwd.c_str());

	dbHelper.BindResultColumnInt(&resultId);

	if (dbHelper.Execute(SQL_PLAYER_LOGIN))
	{
		if(dbHelper.FetchRow())
		{
			printf_s("[INFO] Player Login Completed!\n");
			return resultId == 0;
		}
	}

	return false;
}
