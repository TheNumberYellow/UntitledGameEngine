#include "ServerGameState.h"

#include <json.hpp>

void ServerGameState::OnInitialized()
{
    NetworkModule* network = NetworkModule::Get();
    GraphicsModule* graphics = GraphicsModule::Get();

    //network->StartServer();

    ViewportBuffer = graphics->CreateGBuffer(Vec2i(800, 600));
    graphics->InitializeDebugDraw(ViewportBuffer.FinalOutput);
}

void ServerGameState::OnUninitialized()
{
}

void ServerGameState::OnEnter()
{
}

void ServerGameState::OnExit()
{
}

void ServerGameState::Update(double DeltaTime)
{
    InputModule* input = InputModule::Get();
    UIModule* ui = UIModule::Get();
    NetworkModule* network = NetworkModule::Get();
    GraphicsModule* graphics = GraphicsModule::Get();

    ClientPacket packet;
    //while (network->ServerPollData(packet))
    //{
    //    if (packet.packet.Data._Starts_with("Name:"))
    //    {
    //        ClientNames[packet.id] = packet.packet.Data.substr(5);
    //    }
    //    else
    //    {
    //        std::string NamedMessage = ClientNames[packet.id] + ": " + packet.packet.Data;
    //        ReceivedMessages.push_back(NamedMessage);
    //        network->ServerSendDataAll(NamedMessage);
    //    }
    //}

    if (true)
    {
        ui->StartFrame("Levels", Vec2f(500.0f, 800.0f), 12.0f, MakeColour(125, 200, 255));
        {
            std::string path = "levels";

            for (const auto& entry : std::filesystem::directory_iterator(path))
            {
                std::filesystem::path ext = entry.path().extension();
                if (ext.string() == ".lvl")
                {
                    std::string fileName = entry.path().generic_string();

                    if (ui->TextButton(fileName, Vec2f(400.0f, 50.0f), 8.0f, MakeColour(200, 100, 75)))
                    {
                        SendLevelChangePacket(fileName);
                        CurrentScene.Load(fileName);
                        CurrentScene.Initialize();
                        ViewportCamera = CurrentScene.GetCamera();

                        ViewportCamera->SetScreenSize(Vec2f(800.0f, 600.0f));

                        CurrentScene.SetDirectionalLight(DirectionalLight{ Math::normalize(Vec3f(0.5f, 1.0f, -1.0f)), Vec3f(1.0f, 1.0f, 1.0f) });

                        InScene = true;
                    }
                }
            }
        }
        ui->EndFrame();

        ui->StartFrame("Messages", Vec2f(500.0f, 800.0f), 12.0f, MakeColour(200, 235, 255));
        {
            for (std::string& m : ReceivedMessages)
            {
                ui->TextButton(m, Vec2f(400.0f, 50.0f), 8.0f, MakeColour(255, 235, 200));
            }
        }
        ui->EndFrame();
    }


    if (InScene)
    {
        CurrentScene.Update(DeltaTime);
        CurrentScene.Draw(*graphics, ViewportBuffer);
        graphics->ResetFrameBuffer();
    }

    ui->BufferPanel(ViewportBuffer.FinalOutput, Rect(Vec2f(1000.0f, 0.0f), Vec2f(800.0f, 600.0f)));
}

void ServerGameState::OnResize()
{
}

void ServerGameState::SendLevelChangePacket(std::string levelName)
{
    NetworkModule* network = NetworkModule::Get();

    json PacketData;

    PacketData["Type"] = "LvlChange";
    PacketData["Lvl"] = levelName;

    std::string packetStr = PacketData.dump();

    network->ServerSendDataAll(packetStr);

}
