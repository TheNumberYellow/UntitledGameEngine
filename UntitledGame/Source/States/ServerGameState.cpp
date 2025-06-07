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
    SphereModelPrototype = graphics->CreateModel(*Registry->LoadStaticMesh("Assets/models/UVBall.obj"), graphics->CreateMaterial(*Registry->LoadTexture("Assets/textures/marble.jpg")));
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

            std::string typeStr = PacketData["Type"];
            if (typeStr == "Input")
            {
                json DataPacket = PacketData["Data"];

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

                    if (ui->TextButton(fileName, Vec2f(400.0f, 50.0f), 8.0f, MakeColour(200, 100, 75)))
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

        CurrentScene.Draw(*graphics, ViewportBuffer);
        graphics->ResetFrameBuffer();
    }

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

void ServerGameState::SendSceneUpdatePacket()
{
    NetworkModule* network = NetworkModule::Get();

    json PacketData;
    PacketData["Type"] = "SceneUpdate";
    
    json ModelList;
    
    for (auto& it : CurrentScene.m_Models)
    {
        //for (auto& it : CurrentScene.m_Models)
        {
            json ModelInfo;
            ModelInfo["ID"] = it.first;

            Mat4x4f ModTrans = it.second->GetTransform().GetTransformMatrix();

            ModelInfo["Transform"] = {
                ModTrans[0].x, ModTrans[0].y, ModTrans[0].z, ModTrans[0].w,
                ModTrans[1].x, ModTrans[1].y, ModTrans[1].z, ModTrans[1].w,
                ModTrans[2].x, ModTrans[2].y, ModTrans[2].z, ModTrans[2].w,
                ModTrans[3].x, ModTrans[3].y, ModTrans[3].z, ModTrans[3].w,
            };

            ModelList.push_back(ModelInfo);
        }
    }

    PacketData["Models"] = ModelList;

    std::string packetStr = PacketData.dump();

    network->ServerSendDataAll(packetStr);


    json CamPacket;
    CamPacket["Type"] = "Cam";

    //CamPacket["CamPos"] = { ViewportCamera->GetPosition().x, ViewportCamera->GetPosition().y, ViewportCamera->GetPosition().z };
    //CamPacket["CamDir"] = { ViewportCamera->GetDirection().x, ViewportCamera->GetDirection().y, ViewportCamera->GetDirection().z };

    CamPacket["CamPos"] = { 0.0f, -16.0f, 22.0f };
    CamPacket["CamDir"] = { 0.0f, 0.35f, -0.7f };

    std::string camPacketStr = CamPacket.dump();

    network->ServerSendDataAll(camPacketStr);
}

void ServerGameState::SpawnSphere(ClientID id)
{
    GraphicsModule* graphics = GraphicsModule::Get();
    NetworkModule* network = NetworkModule::Get();

    Model* NewSphereModel = new Model(graphics->CloneModel(SphereModelPrototype));
    NewSphereModel->GetTransform().SetPosition(Vec3f(0.0f, 0.0f, 5.0f));

    NewSphereModel = CurrentScene.AddModel(NewSphereModel);

    SphereController* NewBehaviour = static_cast<SphereController*>(BehaviourRegistry::Get()->AttachNewBehaviour("SphereController", NewSphereModel));
    NewBehaviour->Initialize(&CurrentScene);

    NewBehaviour->SetSystemInputState(&ClientInputStates[id]);

    //SphereController* SphereBehaviour = static_cast<SphereController*>(BehaviourRegistry::Get()->AttachNewBehaviour("SphereController", NewSphereModel));

    //CurrentScene.AddMo


    // Notify clients of new model
    json NewModelPacket;
    NewModelPacket["Type"] = "NewModel";

    NewModelPacket["Mesh"] = "Assets/models/UVBall.obj";
    NewModelPacket["Texture"] = "Assets/textures/marble.jpg";

    network->ServerSendDataAll(NewModelPacket.dump());
}
