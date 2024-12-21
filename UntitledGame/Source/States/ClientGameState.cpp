#include "ClientGameState.h"

#include <json.hpp>

void ClientGameState::OnInitialized(ArgsList args)
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
        // Send inputs
        SendInputPacket();

        //CurrentScene.Update(DeltaTime);
        CurrentScene.Draw(*graphics, ViewportBuffer);
        graphics->ResetFrameBuffer();
    }

    ui->BufferPanel(ViewportBuffer.FinalOutput, Rect(Vec2f(1000.0f, 0.0f), Vec2f(800.0f, 600.0f)));
}

void ClientGameState::OnResize()
{
}

void ClientGameState::SendInputPacket()
{
    NetworkModule* network = NetworkModule::Get();
    InputModule* input = InputModule::Get();

    json PacketData;
    PacketData["Type"] = "Input";

    json KeyData;

    std::vector<std::string> keys;

    KeyState wKeyState = input->GetKeyState(Key::W);
    KeyState aKeyState = input->GetKeyState(Key::A);
    KeyState sKeyState = input->GetKeyState(Key::S);
    KeyState dKeyState = input->GetKeyState(Key::D);

    if (wKeyState.justPressed) KeyData.push_back("W+");
    else if (wKeyState.justReleased) KeyData.push_back("W-");

    if (aKeyState.justPressed) KeyData.push_back("A+");
    else if (aKeyState.justReleased) KeyData.push_back("A-");

    if (sKeyState.justPressed) KeyData.push_back("S+");
    else if (sKeyState.justReleased) KeyData.push_back("S-");

    if (dKeyState.justPressed) KeyData.push_back("D+");
    else if (dKeyState.justReleased) KeyData.push_back("D-");

    PacketData["Data"] = KeyData;

    network->ClientSendData(PacketData.dump());
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

            InScene = true;
        }
        if (typeStr == "SceneUpdate" && InScene)
        {
            json ModelList = PacketData["Models"];

            for (auto& ModelInfo : ModelList)
            {
                GUID modelID = ModelInfo["ID"];

                auto& Trans = ModelInfo["Transform"];
                
                Mat4x4f TransMat;
                TransMat[0] = { Trans[0], Trans[1], Trans[2], Trans[3] };
                TransMat[1] = { Trans[4], Trans[5], Trans[6], Trans[7] };
                TransMat[2] = { Trans[8], Trans[9], Trans[10], Trans[11] };
                TransMat[3] = { Trans[12], Trans[13], Trans[14], Trans[15] };

                CurrentScene.m_Models[modelID]->GetTransform().SetTransformMatrix(TransMat);
            }
        }
        if (typeStr == "Cam" && InScene)
        {
            json CamPos = PacketData["CamPos"];
            json CamDir = PacketData["CamDir"];

            Vec3f PosVec = Vec3f(CamPos[0], CamPos[1], CamPos[2]);
            Vec3f DirVec = Vec3f(CamDir[0], CamDir[1], CamDir[2]);

            CurrentScene.GetCamera()->SetPosition(PosVec);
            CurrentScene.GetCamera()->SetDirection(DirVec);
        }
        if (typeStr == "NewModel" && InScene)
        {
            AssetRegistry* Registry = AssetRegistry::Get();
            GraphicsModule* Graphics = GraphicsModule::Get();

            Model NewModel = Graphics->CreateModel(TexturedMesh(*Registry->LoadStaticMesh(PacketData["Mesh"].get<std::string>()), Graphics->CreateMaterial(*Registry->LoadTexture(PacketData["Texture"].get<std::string>()))));

            CurrentScene.AddModel(new Model(NewModel));

        }
    }
    // Temp to support unstructured packets
    else
    {
        ReceivedMessages.push_back(data);
    }
}
