#include "pch.h"
#include "IocpManager.h"
#include "WorldSession.h"
#include "WorldPacketHandler.h"
#include "MapManager.h"

int main()
{
	WorldPacketHandler::Init();
	GMapManager.Init();
	std::this_thread::sleep_for(1s);

	IocpManager* iocpManager = new IocpManager(L"127.0.0.1", 8888, 1);
	iocpManager->Initialize();
	iocpManager->StartWorker();
	iocpManager->StartConnect<WorldSession>();

	while (true)
	{

	}

	iocpManager->Join();
}
