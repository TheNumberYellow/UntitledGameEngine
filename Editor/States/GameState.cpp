#include "GameState.h"

void GameState::OnInitialized()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    InputModule* Input = InputModule::Get();

    ViewportBuffer = Graphics->CreateGBuffer(Engine::GetClientAreaSize());

    Graphics->InitializeDebugDraw(ViewportBuffer.FinalOutput);

    Rect ViewportRect = GetViewportRect();
    Input->SetMouseCenter(ViewportRect.Center());

    TestFont = TextModule::Get()->LoadFont("fonts/ARLRDBD.TTF", 30);
}

void GameState::OnUninitialized()
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    RuntimeScene.Clear();
    Graphics->DeleteGBuffer(ViewportBuffer);
}

void GameState::OnEnter()
{
}

void GameState::OnExit()
{
}

void GameState::Update(float DeltaTime)
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();
    InputModule* Input = InputModule::Get();

    RuntimeScene.Update(DeltaTime);

    RuntimeScene.Draw(*Graphics, ViewportBuffer);

    Graphics->ResetFrameBuffer();

    UI->BufferPanel(ViewportBuffer.FinalOutput, GetViewportRect());

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


    if (Input->GetKeyState(Key::Escape))
    {
        Machine->PopState();
    }
}

void GameState::OnResize()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    InputModule* Input = InputModule::Get();

    Rect ViewportRect = GetViewportRect();
    
    ViewportCamera->SetScreenSize(ViewportRect.size);
    Graphics->ResizeGBuffer(ViewportBuffer, ViewportRect.size);
    Input->SetMouseCenter(ViewportRect.Center());
}

void GameState::LoadScene(Scene& InScene)
{
    RuntimeScene = Scene(InScene);
    RuntimeScene.Initialize();

    ViewportCamera = RuntimeScene.GetCamera();
    ViewportCamera->SetScreenSize(GetViewportRect().size);
}

Rect GameState::GetViewportRect()
{
    Rect Result;
    Result.size = Engine::GetClientAreaSize();
    
    return Result;
}
