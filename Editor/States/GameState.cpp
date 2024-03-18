#include "GameState.h"

void GameState::OnInitialized()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    InputModule* Input = InputModule::Get();

    ViewportBuffer = Graphics->CreateGBuffer(Engine::GetClientAreaSize());

    Graphics->InitializeDebugDraw(ViewportBuffer.FinalOutput);

    ViewportCamera = Camera(Projection::Perspective);
    ViewportCamera.SetScreenSize(GetViewportRect().size);

    Rect ViewportRect = GetViewportRect();
    Input->SetMouseCenter(ViewportRect.Center());
}

void GameState::OnUninitialized()
{
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
    
    ViewportCamera.SetScreenSize(ViewportRect.size);
    Graphics->ResizeGBuffer(ViewportBuffer, ViewportRect.size);
    Input->SetMouseCenter(ViewportRect.Center());
}

void GameState::LoadScene(Scene& InScene)
{
    RuntimeScene = Scene(InScene);

    RuntimeScene.SetCamera(&ViewportCamera);
}

Rect GameState::GetViewportRect()
{
    Rect Result;
    Result.size = Engine::GetClientAreaSize();
    
    return Result;
}
