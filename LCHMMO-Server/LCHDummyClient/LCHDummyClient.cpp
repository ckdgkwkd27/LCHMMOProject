#include "pch.h"
#include "SocketUtil.h"
#include "SessionManager.h"
#include "ServerSession.h"
#include "IocpManager.h"
#include "ServerPacketHandler.h"
#include "RandomUtil.h"
#include "Ticker.h"

int main()
{
    ServerPacketHandler::Init();

    IocpManager* iocpManager = new IocpManager(L"127.0.0.1", 8888, 1500);
    iocpManager->Initialize();
    iocpManager->StartWorker();
    iocpManager->StartConnect<ServerSession>();

    std::this_thread::sleep_for(5s);
    std::cout << "INFO: Tick Thread Started.." << std::endl;

	std::thread tickThread = std::thread([]
	{
		Ticker ticker([]() 
            {
                for (auto& session : GSessionManager.activeSessions)
                {
                    ServerSessionPtr playerSession = std::dynamic_pointer_cast<ServerSession>(session);
                    if (playerSession->state == ENTER_GAME)
                    {
                        float prob = RandomUtil::GetRandomFloat();
                        uint32 dir;
                        if (prob >= 0.25)
                        {
                            playerSession->posX = playerSession->posX + 1;
                            dir = 4;
                        }
                        else if (prob > 0.25 && prob < 0.5)
                        {
                            playerSession->posX = playerSession->posX - 1;
                            dir = 3;
                        }
                        else if (prob >= 0.5 && prob < 0.75)
                        {
                            playerSession->posY = playerSession->posY + 1;
                            dir = 2;
                        }
                        else
                        {
                            playerSession->posY = playerSession->posY - 1;
                            dir = 1;
                        }

                        protocol::RequestMove movePkt;
                        movePkt.mutable_posinfo()->set_posx(playerSession->posX);
                        movePkt.mutable_posinfo()->set_posy(playerSession->posY);
                        movePkt.mutable_posinfo()->set_state(1);
                        movePkt.mutable_posinfo()->set_movedir(dir);
                        auto _sendBuffer = ServerPacketHandler::MakeSendBufferPtr(movePkt);
                        if (playerSession->PostSend(_sendBuffer) == false)
                            return;
                    }
                }
            }, std::chrono::duration<int64, std::milli>(200));
	});

    while (true)
    {

    }

    iocpManager->Join();

    return true;
}
