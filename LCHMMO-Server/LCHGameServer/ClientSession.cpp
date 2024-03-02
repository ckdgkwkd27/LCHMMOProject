#include "pch.h"
#include "ClientSession.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "Zone.h"
#include "ZoneManager.h"

void ClientSession::OnAccepted()
{
	GSessionManager.AddToActivePool(shared_from_this());
}

void ClientSession::OnConnected()
{
	std::cout << "OnConnected()" << std::endl;
}

void ClientSession::OnDisconnected()
{
	GSessionManager.DeleteFromActivePool(shared_from_this());

	if(currentPlayer != nullptr)
	{
		ZonePtr zone = GZoneManager.FindZoneByID(this->currentPlayer->zoneID);
		zone->messageQueue.Push([=]() {zone->LeaveGame(this->currentPlayer->ActorInfo.actorid()); });
	}
}

uint32 ClientSession::OnRecv(char* buffer, uint32 len)
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

		ClientPacketHandler::HandlePacket(std::dynamic_pointer_cast<ClientSession>(shared_from_this()), buffer, len);
		processLen += header.size;
	}

	return processLen;
}
