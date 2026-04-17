#include "ServerGameState.h"
#include "Behaviours/SphereController.h"

#include <json.hpp>

void ServerGameState::OnInitialized(ArgsList args)
{
    NetworkModule* network = NetworkModule::Get();
    GraphicsModule* graphics = GraphicsModule::Get();
    InputModule* input = InputModule::Get();

    network->StartServer();

    ViewportBuffer = graphics->CreateGBuffer(Vec2i(800, 600));
    graphics->InitializeDebugDraw(ViewportBuffer.FinalOutput);

    AssetRegistry* Registry = AssetRegistry::Get();
    SphereModelPrototype = graphics->CreateModel(*Registry->LoadStaticMesh("Assets/models/UVBall.obj"), graphics->CreateMaterial(Registry->LoadTexture("Assets/textures/marble.jpg")));
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
    while (network->ServerPollData(packet))
    {
        ClientID id = packet.id;

        if (json::accept(packet.packet.Data))
        {
            json PacketData = json::parse(packet.packet.Data);

            std::string typeStr = PacketData["T"];
            if (typeStr == "I")
            {
                json DataPacket = PacketData["D"];

                for (json& InputData : DataPacket)
                {
                    std::string inputString = InputData;

                    SystemInputState& clientInputState = ClientInputStates[packet.id];

                    if (inputString == "W+") clientInputState.SetKeyDown(Key::W, true);
                    if (inputString == "W-") clientInputState.SetKeyDown(Key::W, false);
                    if (inputString == "A+") clientInputState.SetKeyDown(Key::A, true);
                    if (inputString == "A-") clientInputState.SetKeyDown(Key::A, false);
                    if (inputString == "S+") clientInputState.SetKeyDown(Key::S, true);
                    if (inputString == "S-") clientInputState.SetKeyDown(Key::S, false);
                    if (inputString == "D+") clientInputState.SetKeyDown(Key::D, true);
                    if (inputString == "D-") clientInputState.SetKeyDown(Key::D, false);
                    if (inputString == "SP+") clientInputState.SetKeyDown(Key::Space, true);
                    if (inputString == "SP-") clientInputState.SetKeyDown(Key::Space, false);
                    if (inputString == "R+") clientInputState.SetKeyDown(Key::R, true);
                    if (inputString == "R-") clientInputState.SetKeyDown(Key::R, false);

                    if (inputString.starts_with("M:"))
                    {
                        std::string mouseData = inputString.substr(2);
                        size_t commaPos = mouseData.find(',');
                        if (commaPos != std::string::npos)
                        {
                            std::string xStr = mouseData.substr(0, commaPos);
                            std::string yStr = mouseData.substr(commaPos + 1);
                            int x = std::stoi(xStr);
                            int y = std::stoi(yStr);
                            clientInputState.GetMouseState().SetDeltaPos(Vec2i(x, y));
                            
                        }
                    }

                    GamepadState& gamepadState = clientInputState.GetGamepadState();

                    if (inputString == "G_E+")
                    {
                        gamepadState.SetEnabled(true);
                    }
                    else if (inputString == "G_E-")
                    {
                        gamepadState.SetEnabled(false);
                    }

                    if (gamepadState.IsEnabled())
                    {
                        if (inputString == "GN+") gamepadState.SetButtonDown(Button::Face_North, true);
                        if (inputString == "GN-") gamepadState.SetButtonDown(Button::Face_North, false);
                        if (inputString == "GE+") gamepadState.SetButtonDown(Button::Face_East, true);
                        if (inputString == "GE-") gamepadState.SetButtonDown(Button::Face_East, false);
                        if (inputString == "GS+") gamepadState.SetButtonDown(Button::Face_South, true);
                        if (inputString == "GS-") gamepadState.SetButtonDown(Button::Face_South, false);
                        if (inputString == "GW+") gamepadState.SetButtonDown(Button::Face_West, true);
                        if (inputString == "GW-") gamepadState.SetButtonDown(Button::Face_West, false);

                        if (inputString.starts_with("GL:"))
                        {
                            std::string stickData = inputString.substr(3);
                            size_t commaPos = stickData.find(',');
                            if (commaPos != std::string::npos)
                            {
                                std::string xStr = stickData.substr(0, commaPos);
                                std::string yStr = stickData.substr(commaPos + 1);
                                float x = std::stof(xStr);
                                float y = std::stof(yStr);

                                gamepadState.UpdateLeftStickAxis(Vec2f(x, y));
                            }
                        }

                        if (inputString.starts_with("GR:"))
                        {
                            std::string stickData = inputString.substr(3);
                            size_t commaPos = stickData.find(',');
                            if (commaPos != std::string::npos)
                            {
                                std::string xStr = stickData.substr(0, commaPos);
                                std::string yStr = stickData.substr(commaPos + 1);
                                float x = std::stof(xStr);
                                float y = std::stof(yStr);

                                gamepadState.UpdateRightStickAxis(Vec2f(x, y));
                            }
                        }
                    }
                }
            }
        }
        else if (packet.packet.Data._Starts_with("Name:"))
        {
            // New client
            ClientNames[packet.id] = packet.packet.Data.substr(5);
            ClientInputStates[packet.id] = SystemInputState();

            if (InScene)
            {
                SpawnSphere(packet.id);
            }
        }
        else
        {
            // Client message (TODO: handle this as json)
            std::string NamedMessage = ClientNames[packet.id] + ": " + packet.packet.Data;
            ReceivedMessages.push_back(NamedMessage);
            network->ServerSendDataAll(NamedMessage);
        }
    }

    ui->BufferPanel(ViewportBuffer.FinalOutput, Vec2f(800.0f, 600.0f));
    
    if (true)
    {
        ui->StartFrame("Levels", Vec2f(500.0f, 800.0f), 12.0f, MakeColour(125, 200, 255));
        {
            std::string path = "Assets/levels";

            for (const auto& entry : std::filesystem::directory_iterator(path))
            {
                std::filesystem::path ext = entry.path().extension();
                if (ext.string() == ".lvl")
                {
                    std::string fileName = entry.path().generic_string();

                    if (ui->TextButton(fileName, PlacementType::FIT_WIDTH, 8.0f, MakeColour(200, 100, 75)))
                    {
                        SendLevelChangePacket(fileName);
                        CurrentScene.Load(fileName);
                        CurrentScene.Initialize();
                        ViewportCamera = CurrentScene.GetCamera();

                        ViewportCamera->SetScreenSize(Vec2f(800.0f, 600.0f));

                        InScene = true;

                        for (auto& it : ClientInputStates)
                        {
                            SpawnSphere(it.first);
                        }
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

        SendSceneUpdatePacket();

        //CurrentScene.Draw(*graphics, ViewportBuffer);
        graphics->ResetFrameBuffer();

        if (ui->TextButton("Return to Lobby"))
        {
            SendReturnToLobbyPacket();
            InScene = false;
        }
    }

}

void ServerGameState::OnResize()
{
}

void ServerGameState::SendLevelChangePacket(std::string levelName)
{
    NetworkModule* network = NetworkModule::Get();

    json PacketData;

    PacketData["T"] = "LC";
    PacketData["L"] = levelName;

    std::string packetStr = PacketData.dump();

    network->ServerSendDataAll(packetStr);
}

void ServerGameState::SendReturnToLobbyPacket()
{
    NetworkModule* network = NetworkModule::Get();

    json PacketData;

    PacketData["T"] = "RTL";

    std::string packetStr = PacketData.dump();

    network->ServerSendDataAll(packetStr);
}

void ServerGameState::SendSceneUpdatePacket()
{
    NetworkModule* network = NetworkModule::Get();

    json PacketData;
    PacketData["T"] = "SU";
    
    json ModelList;
    
    for (auto& it : CurrentScene.m_Models)
    {
        //for (auto& it : CurrentScene.m_Models)
        {
            json ModelInfo;
            ModelInfo["ID"] = it.first;

            Mat4x4f ModTrans = it.second->GetTransform().GetTransformMatrix();

            ModelInfo["T"] = {
                ModTrans[0].x, ModTrans[0].y, ModTrans[0].z, ModTrans[0].w,
                ModTrans[1].x, ModTrans[1].y, ModTrans[1].z, ModTrans[1].w,
                ModTrans[2].x, ModTrans[2].y, ModTrans[2].z, ModTrans[2].w,
                ModTrans[3].x, ModTrans[3].y, ModTrans[3].z, ModTrans[3].w,
            };

            ModelList.push_back(ModelInfo);
        }
    }

    PacketData["ML"] = ModelList;

    std::string packetStr = PacketData.dump();

    network->ServerSendDataAll(packetStr);

    for (auto& it : ClientNames)
    {
        SendCameraInfo(it.first);
    }

    //json CamPacket;
    //CamPacket["T"] = "C";

    ////CamPacket["CamPos"] = { ViewportCamera->GetPosition().x, ViewportCamera->GetPosition().y, ViewportCamera->GetPosition().z };
    ////CamPacket["CamDir"] = { ViewportCamera->GetDirection().x, ViewportCamera->GetDirection().y, ViewportCamera->GetDirection().z };

    //// Cam position
    //CamPacket["CP"] = { 0.0f, -16.0f, 22.0f };
    //// Cam direction
    //CamPacket["CD"] = { 0.0f, 0.35f, -0.7f };

    //std::string camPacketStr = CamPacket.dump();

    //network->ServerSendDataAll(camPacketStr);
}

void ServerGameState::SpawnSphere(ClientID id)
{
    GraphicsModule* graphics = GraphicsModule::Get();
    NetworkModule* network = NetworkModule::Get();

    Model* NewSphereModel = new Model(graphics->CloneModel(SphereModelPrototype));
    NewSphereModel->GetTransform().SetPosition(Vec3f(0.0f, 0.0f, 5.0f));

    NewSphereModel = CurrentScene.AddModel(NewSphereModel);

    SphereController* NewBehaviour = static_cast<SphereController*>(BehaviourRegistry::Get()->AttachNewBehaviour("SphereController", NewSphereModel));
    NewBehaviour->SetRunningLocally(false);
    NewBehaviour->Initialize(&CurrentScene);

    NewBehaviour->SetSystemInputState(&ClientInputStates[id]);
    
    Camera* NewCamera = new Camera();
    NewBehaviour->SetCamera(NewCamera);
    ClientCameras[id] = NewCamera;


    // Notify clients of new model
    json NewModelPacket;
    NewModelPacket["T"] = "NM";

    NewModelPacket["Mesh"] = "Assets/models/UVBall.obj";

    std::string playerName = ClientNames[id];
    std::transform(playerName.begin(), playerName.end(), playerName.begin(), ::tolower);

    if (playerName == "fraser")
    {
        NewModelPacket["TextureA"] = "Assets/textures/PencilBlack.png";
        NewModelPacket["TextureN"] = "Assets/textures/PencilBlack.norm.png";
        NewModelPacket["TextureR"] = "Assets/textures/PencilBlack.rough.png";
        NewModelPacket["TextureM"] = "Assets/textures/PencilBlack.metal.png";
    }
    else if (playerName == "erik")
    {
        NewModelPacket["TextureA"] = "Assets/textures/rusted-grate.png";
        NewModelPacket["TextureN"] = "Assets/textures/rusted-grate.norm.png";
        NewModelPacket["TextureR"] = "Assets/textures/rusted-grate.rough.png";
        NewModelPacket["TextureM"] = "Assets/textures/rusted-grate.metal.png";
        NewModelPacket["TextureO"] = "Assets/textures/rusted-grate.ao.png";
    }
    else if (playerName == "shadow")
    {
        NewModelPacket["TextureA"] = "Assets/textures/pirate-gold.png";
        NewModelPacket["TextureN"] = "Assets/textures/pirate-gold.norm.png";
        NewModelPacket["TextureR"] = "Assets/textures/pirate-gold.rough.png";
        NewModelPacket["TextureM"] = "Assets/textures/pirate-gold.metal.png";
        NewModelPacket["TextureO"] = "Assets/textures/pirate-gold.ao.png";
    }
    else
    {
        NewModelPacket["TextureA"] = "Assets/textures/marble.jpg";
        NewModelPacket["TextureN"] = "Assets/textures/marble.norm.png";
    }

    network->ServerSendDataAll(NewModelPacket.dump());
}

void ServerGameState::SendCameraInfo(ClientID id)
{
    if (Camera* clientCamera = ClientCameras[id])
    {
        NetworkModule* network = NetworkModule::Get();

        json CamPacket;

        Vec3f camPos = clientCamera->GetPosition();
        Vec3f camDir = clientCamera->GetDirection();

        CamPacket["T"] = "C";
        CamPacket["CP"] = { camPos.x, camPos.y, camPos.z };
        CamPacket["CD"] = { camDir.x, camDir.y, camDir.z };

        network->ServerSendData(CamPacket.dump(), id);
    }
}
