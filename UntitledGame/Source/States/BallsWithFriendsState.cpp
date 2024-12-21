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
    if (ui->TextButton("Client", Vec2f(200.0f, 80.0f), 8.0f))
    {
        ClientGameState* ClientState = new ClientGameState();
        Machine->PushState(ClientState);
    }
    if (ui->TextButton("Server", Vec2f(200.0f, 80.0f), 8.0f))
    {
        ServerGameState* ServerState = new ServerGameState();
        Machine->PushState(ServerState);
    }
}

void BallsWithFriendsState::OnResize()
{
}
