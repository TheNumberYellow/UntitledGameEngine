#include "ClientGameState.h"

#include <json.hpp>

void ClientGameState::OnInitialized(ArgsList args)
{
    GraphicsModule* graphics = GraphicsModule::Get();
    InputModule* Input = InputModule::Get();

    ViewportBuffer = graphics->CreateGBuffer(Engine::GetClientAreaSize());
    graphics->InitializeDebugDraw(ViewportBuffer.FinalOutput);

    Rect ViewportRect = GetViewportRect();
    Input->SetMouseCenter(ViewportRect.Center());

    TestFont = TextModule::Get()->LoadFont("Assets/fonts/ARLRDBD.TTF", 30);
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
        ui->StartFrame("Greetings", PlacementType::FIT_BOTH, 8.0f, MakeColour(32, 107, 239), false);
        {
            ui->Text("Name                                         IP");
            ui->NewLine();
            ui->TextEntry("Name", NameEntry, Vec2f(200.0f, 80.0f));
            ui->TextEntry("IP", IPEntry, Vec2f(200.0f, 80.0f));

            if (ui->TextButton("Connect", Vec2f(200.0f, 80.0f), 8.0f))
            {
                if (IPEntry == "") IPEntry = "localhost";

                if (network->StartClient(IPEntry))
                {
                    Connected = true;
            
                    network->ClientSendData("Name:" + NameEntry);
                }
            }
        }
        ui->EndFrame();
    }
    else
    {
        Packet packet;
        while (network->ClientPollData(packet))
        {
            ProcessPacketData(packet.Data);
        }

        if (!InScene)
        {
            ui->StartFrame("Messages", Vec2f(500.0f, 800.0f), 12.0f, MakeColour(100, 235, 255));
            {
                for (std::string& m : ReceivedMessages)
                {
                    ui->Text(m);
                    ui->NewLine();
                    //ui->TextButton(m, Vec2f(400.0f, 50.0f), 8.0f, MakeColour(155, 185, 190));
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
    }

    if (InScene)
    {
        // Send inputs
        SendInputPacket();

        //CurrentScene.Update(DeltaTime);
        CurrentScene.Draw(*graphics, ViewportBuffer);
        graphics->ResetFrameBuffer();
        
        Rect viewportRect = GetViewportRect();

        ui->BufferPanel(ViewportBuffer.FinalOutput, viewportRect);
        
        TextModule::Get()->DrawText("Frame Time: " + std::to_string(DeltaTime), &TestFont, Vec2f(0.0f, 0.0f));

        PrevFrameTimeCount++;
        PrevFrameTimeSum += DeltaTime;

        if (PrevFrameTimeSum > 0.5f)
        {
            PrevAveFPS = (int)round(1.0f / (PrevFrameTimeSum / PrevFrameTimeCount));
            PrevFrameTimeCount = 0;
            PrevFrameTimeSum -= 0.5f;
        }

        TextModule::Get()->DrawText("FPS: " + std::to_string(PrevAveFPS), &TestFont, Vec2f(0.0f, 30.0f));

    }

}

void ClientGameState::OnResize()
{
    if (InScene)
    {
        GraphicsModule* Graphics = GraphicsModule::Get();
        InputModule* Input = InputModule::Get();

        Rect ViewportRect = GetViewportRect();

        ViewportCamera->SetScreenSize(ViewportRect.size);
        Graphics->ResizeGBuffer(ViewportBuffer, ViewportRect.size);
        Input->SetMouseCenter(ViewportRect.Center());
    }
}

Rect ClientGameState::GetViewportRect()
{
    Rect Result;
    Result.size = Engine::GetClientAreaSize();

    return Result;
}

void ClientGameState::SendInputPacket()
{
    NetworkModule* network = NetworkModule::Get();
    InputModule* input = InputModule::Get();

    json PacketData;
    PacketData["T"] = "I";

    json KeyData;

    std::vector<std::string> keys;

    KeyState wKeyState = input->GetKeyState(Key::W);
    KeyState aKeyState = input->GetKeyState(Key::A);
    KeyState sKeyState = input->GetKeyState(Key::S);
    KeyState dKeyState = input->GetKeyState(Key::D);
    KeyState spaceKeyState = input->GetKeyState(Key::Space);
    KeyState rKeyState = input->GetKeyState(Key::R);

    if (wKeyState.justPressed) KeyData.push_back("W+");
    else if (wKeyState.justReleased) KeyData.push_back("W-");

    if (aKeyState.justPressed) KeyData.push_back("A+");
    else if (aKeyState.justReleased) KeyData.push_back("A-");

    if (sKeyState.justPressed) KeyData.push_back("S+");
    else if (sKeyState.justReleased) KeyData.push_back("S-");

    if (dKeyState.justPressed) KeyData.push_back("D+");
    else if (dKeyState.justReleased) KeyData.push_back("D-");

    if (spaceKeyState.justPressed) KeyData.push_back("SP+");
    else if (spaceKeyState.justReleased) KeyData.push_back("SP-");

    if (rKeyState.justPressed) KeyData.push_back("R+");
    else if (rKeyState.justReleased) KeyData.push_back("R-");


    Vec2i deltaMousePos = input->GetMouseState().GetDeltaMousePos();
    KeyData.push_back("M:" + std::to_string(deltaMousePos.x) + "," + std::to_string(deltaMousePos.y));

    if (input->GetGamepadState().IsEnabled())
    {
        KeyData.push_back("G_E+");
        GamepadState& gPadState = input->GetGamepadState();
        
        KeyState northButtonState = gPadState.GetButtonState(Button::Face_North);
        KeyState eastButtonState = gPadState.GetButtonState(Button::Face_East);
        KeyState southButtonState = gPadState.GetButtonState(Button::Face_South);
        KeyState westButtonState = gPadState.GetButtonState(Button::Face_West);
        
        if (northButtonState.justPressed) KeyData.push_back("GN+");
        else if (northButtonState.justReleased) KeyData.push_back("GN-");

        if (eastButtonState.justPressed) KeyData.push_back("GE+");
        else if (eastButtonState.justReleased) KeyData.push_back("GE-");

        if (southButtonState.justPressed) KeyData.push_back("GS+");
        else if (southButtonState.justReleased) KeyData.push_back("GS-");

        if (westButtonState.justPressed) KeyData.push_back("GW+");
        else if (westButtonState.justReleased) KeyData.push_back("GW-");

        Vec2f leftStickAxis = gPadState.GetLeftStickAxis();
        Vec2f rightStickAxis = gPadState.GetRightStickAxis();

        KeyData.push_back("GL:" + std::to_string(leftStickAxis.x) + "," + std::to_string(leftStickAxis.y));
        KeyData.push_back("GR:" + std::to_string(rightStickAxis.x) + "," + std::to_string(rightStickAxis.y));
    }
    else {
        KeyData.push_back("G_E-");
    }

    PacketData["D"] = KeyData;

    network->ClientSendData(PacketData.dump());
}

void ClientGameState::ProcessPacketData(const std::string& data)
{
    if (json::accept(data))
    {
        json PacketData = json::parse(data);

        std::string typeStr = PacketData["T"];

        if (typeStr == "LC")
        {
            //Engine::LockCursor();
            //Engine::HideCursor();

            CurrentScene.Load(PacketData["L"].get<std::string>());
            CurrentScene.Initialize();
            ViewportCamera = CurrentScene.GetCamera();

            ViewportCamera->SetScreenSize(GetViewportRect().size);

            InScene = true;
        }
        if (typeStr == "SU" && InScene)
        {
            json ModelList = PacketData["ML"];

            for (auto& ModelInfo : ModelList)
            {
                uGUID modelID = ModelInfo["ID"];

                auto& Trans = ModelInfo["T"];
                
                Mat4x4f TransMat;
                TransMat[0] = { Trans[0], Trans[1], Trans[2], Trans[3] };
                TransMat[1] = { Trans[4], Trans[5], Trans[6], Trans[7] };
                TransMat[2] = { Trans[8], Trans[9], Trans[10], Trans[11] };
                TransMat[3] = { Trans[12], Trans[13], Trans[14], Trans[15] };

                CurrentScene.m_Models[modelID]->GetTransform().SetTransformMatrix(TransMat);
            }
        }
        if (typeStr == "C" && InScene)
        {
            json CamPos = PacketData["CP"];
            json CamDir = PacketData["CD"];

            Vec3f PosVec = Vec3f(CamPos[0], CamPos[1], CamPos[2]);
            Vec3f DirVec = Vec3f(CamDir[0], CamDir[1], CamDir[2]);

            CurrentScene.GetCamera()->SetPosition(PosVec);
            CurrentScene.GetCamera()->SetDirection(DirVec);
        }
        if (typeStr == "NM" && InScene)
        {
            AssetRegistry* Registry = AssetRegistry::Get();
            GraphicsModule* Graphics = GraphicsModule::Get();

            if (PacketData.contains("TextureO"))
            {
                Model NewModel = Graphics->CreateModel(
                    *Registry->LoadStaticMesh(PacketData["Mesh"].get<std::string>()),
                    Graphics->CreateMaterial(
                        Registry->LoadTexture(PacketData["TextureA"].get<std::string>()),
                        Registry->LoadTexture(PacketData["TextureN"].get<std::string>()),
                        Registry->LoadTexture(PacketData["TextureR"].get<std::string>()),
                        Registry->LoadTexture(PacketData["TextureM"].get<std::string>()),
                        Registry->LoadTexture(PacketData["TextureO"].get<std::string>())
                    ));

                CurrentScene.AddModel(new Model(NewModel));
            }
            else if (PacketData.contains("TextureM"))
            {
                Model NewModel = Graphics->CreateModel(
                    *Registry->LoadStaticMesh(PacketData["Mesh"].get<std::string>()), 
                    Graphics->CreateMaterial(
                        Registry->LoadTexture(PacketData["TextureA"].get<std::string>()),
                        Registry->LoadTexture(PacketData["TextureN"].get<std::string>()),
                        Registry->LoadTexture(PacketData["TextureR"].get<std::string>()),
                        Registry->LoadTexture(PacketData["TextureM"].get<std::string>())
                    ));

                CurrentScene.AddModel(new Model(NewModel));
            }
            else if (PacketData.contains("TextureR"))
            {
                Model NewModel = Graphics->CreateModel(
                    *Registry->LoadStaticMesh(PacketData["Mesh"].get<std::string>()), 
                    Graphics->CreateMaterial(
                        Registry->LoadTexture(PacketData["TextureA"].get<std::string>()),
                        Registry->LoadTexture(PacketData["TextureN"].get<std::string>()),
                        Registry->LoadTexture(PacketData["TextureR"].get<std::string>())
                    ));

                CurrentScene.AddModel(new Model(NewModel));
            }
            else if (PacketData.contains("TextureN"))
            {
                Model NewModel = Graphics->CreateModel(
                    *Registry->LoadStaticMesh(PacketData["Mesh"].get<std::string>()), 
                    Graphics->CreateMaterial(
                        Registry->LoadTexture(PacketData["TextureA"].get<std::string>()),
                        Registry->LoadTexture(PacketData["TextureN"].get<std::string>())
                    ));

                CurrentScene.AddModel(new Model(NewModel));
            }
            else
            {
                Model NewModel = Graphics->CreateModel(
                    *Registry->LoadStaticMesh(PacketData["Mesh"].get<std::string>()), 
                    Graphics->CreateMaterial(
                        Registry->LoadTexture(PacketData["TextureA"].get<std::string>())
                    ));

                CurrentScene.AddModel(new Model(NewModel));
            }
        }
        if (typeStr == "RTL" && InScene)
        {
            InScene = false;
            Engine::UnlockCursor();
            Engine::ShowCursor();
        }
    }
    // Temp to support unstructured packets
    else
    {
        ReceivedMessages.push_back(data);
    }
}
