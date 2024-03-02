#include "pch.h"
#include "SessionManager.h"
#include "IocpManager.h"
#include "ClientSession.h"
#include "ClientPacketHandler.h"
#include "PlayerManager.h"
#include "ZoneManager.h"
#include "Ticker.h"
#include "DBManager.h"

int main()
{
    GPlayerManager.Initialize();
    GZoneManager.Initialize();

    ClientPacketHandler::Init();

    GIocpManager = new IocpManager(L"127.0.0.1", 8888, 1500);
    GIocpManager->Initialize();
    GIocpManager->BindAndListen();
    CRASH_ASSERT(GIocpManager->StartWorker());

	GDBManager = new DBManager();
	GDBManager->Initialize();
    GDBManager->Start();

    GIocpManager->StartAccept<ClientSession>();

	GZoneManager.SpawnNpc();

    std::thread tickThread = std::thread([] 
	{
		Ticker ticker([]() { GZoneManager.TickUpdate(); }, std::chrono::duration<int64, std::milli>(15));
	});

    std::thread monitorThread = std::thread([]
    {
        Ticker ticker([]() { 
            printf("[MONITOR] SendTPS=%lld\n", GIocpManager->sendTps.load()); 
            printf("[MONITOR] RecvTPS=%lld\n", GIocpManager->recvTps.load());
			GIocpManager->sendTps = 0;
			GIocpManager->recvTps = 0;
        }, std::chrono::duration<int64, std::milli>(1000));
    });


    while (true)
    {

    }

    GIocpManager->Join();
    tickThread.join();
    monitorThread.join();

    return true;
}
