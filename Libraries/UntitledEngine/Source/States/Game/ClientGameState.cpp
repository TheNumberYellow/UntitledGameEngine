#include "ClientGameState.h"

#include <json.hpp>

void ClientGameState::OnInitialized()
{
    GraphicsModule* graphics = GraphicsModule::Get();

    ViewportBuffer = graphics->CreateGBuffer(Vec2i(800, 600));
    graphics->InitializeDebugDraw(ViewportBuffer.FinalOutput);

}

void ClientGameState::OnUninitialized()
{
}

void ClientGameState::OnEnter()
{
}

void ClientGameState::OnExit()
{
}

void ClientGameState::Update(double DeltaTime)
{
    UIModule* ui = UIModule::Get();
    NetworkModule* network = NetworkModule::Get();
    GraphicsModule* graphics = GraphicsModule::Get();

    if (!Connected)
    {
        ui->TextEntry("Name", NameEntry, Vec2f(200.0f, 80.0f));
        ui->TextEntry("IP", IPEntry, Vec2f(200.0f, 80.0f));

        if (ui->TextButton("Connect", Vec2f(200.0f, 80.0f), 8.0f))
        {
            if (network->StartClient(IPEntry))
            {
                Connected = true;
            
                network->ClientSendData("Name:" + NameEntry);
            }
        }
    }
    else
    {
        Packet packet;
        while (network->ClientPollData(packet))
        {
            ProcessPacketData(packet.Data);
        }

        ui->StartFrame("Messages", Vec2f(500.0f, 800.0f), 12.0f, MakeColour(100, 235, 255));
        {
            for (std::string& m : ReceivedMessages)
            {
                ui->TextButton(m, Vec2f(400.0f, 50.0f), 8.0f, MakeColour(155, 185, 190));
            }
        }

        ui->EndFrame();

        ui->TextEntry("Message box", MessageEntry, Vec2f(200.0f, 80.0f));

        if (ui->TextButton("Send Message", Vec2f(200.0f, 80.0f), 8.0f))
        {
            if (MessageEntry != "")
            {
                network->ClientSendData(MessageEntry);
                MessageEntry = "";
            }
        }

        if (ui->TextButton("Ping server", Vec2f(200.0f, 80.0f), 8.0f))
        {
            network->ClientSendData("Ping");
        }

    }

    if (InScene)
    {
        CurrentScene.Update(DeltaTime);
        CurrentScene.Draw(*graphics, ViewportBuffer);
        graphics->ResetFrameBuffer();
    }

    ui->BufferPanel(ViewportBuffer.FinalOutput, Rect(Vec2f(1000.0f, 0.0f), Vec2f(800.0f, 600.0f)));
}

void ClientGameState::OnResize()
{
}

void ClientGameState::ProcessPacketData(const std::string& data)
{
    if (json::accept(data))
    {
        json PacketData = json::parse(data);

        std::string typeStr = PacketData["Type"];

        if (typeStr == "LvlChange")
        {
            CurrentScene.Load(PacketData["Lvl"].get<std::string>());
            CurrentScene.Initialize();
            ViewportCamera = CurrentScene.GetCamera();

            ViewportCamera->SetScreenSize(Vec2f(800.0f, 600.0f));

            CurrentScene.SetDirectionalLight(DirectionalLight{ Math::normalize(Vec3f(0.5f, 1.0f, -1.0f)), Vec3f(1.0f, 1.0f, 1.0f) });

            InScene = true;
        }
    }
    // Temp to support unstructured packets
    else
    {
        ReceivedMessages.push_back(data);
    }
}
