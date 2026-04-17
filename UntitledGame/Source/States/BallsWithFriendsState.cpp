#include "BallsWithFriendsState.h"

#include "ClientGameState.h"
#include "ServerGameState.h"

REGISTER_STATE(BallsWithFriendsState);

void BallsWithFriendsState::OnInitialized(ArgsList args)
{
}

void BallsWithFriendsState::OnUninitialized()
{
}

void BallsWithFriendsState::OnEnter()
{
}

void BallsWithFriendsState::OnExit()
{
}

void BallsWithFriendsState::Update(double DeltaTime)
{
    UIModule* ui = UIModule::Get();
    ui->StartFrame("Selector", PlacementType::FIT_BOTH, 8.0f, MakeColour(25, 230, 150), false);
    {
        Vec2f frameSize = ui->GetCurrentFrameSize();
        if (ui->TextButton("Client", PlacementSettings(PlacementType::FIT_HEIGHT, frameSize.x / 2.0f), 8.0f))
        {
            ClientGameState* ClientState = new ClientGameState();
            Machine->PushState(ClientState);
        }
        if (ui->TextButton("Server", PlacementSettings(PlacementType::FIT_HEIGHT, frameSize.x / 2.0f), 8.0f))
        {
            ServerGameState* ServerState = new ServerGameState();
            Machine->PushState(ServerState);
        }

    }
    ui->EndFrame();
}

void BallsWithFriendsState::OnResize()
{
}
