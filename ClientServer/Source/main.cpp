#include "GameEngine.h"

#include "States/Game/ServerGameState.h"
#include "States/Game/ClientGameState.h"
#include "State/StateMachine.h"

StateMachine Machine;

void Initialize(ArgsList args)
{
    Engine::DEBUGPrint(args);

    // ??????????????
    //AssetRegistry* Registry = AssetRegistry::Get();

    //Registry->LoadStaticMesh("models/ArrowSmooth.obj"),
    //Registry->LoadStaticMesh("models/RotationHoop.obj");


    if (args == "-server")
    {
        ServerGameState* ServerState = new ServerGameState();
        Machine.PushState(ServerState);
    }
    else
    {
        ClientGameState* ClientState = new ClientGameState();
        Machine.PushState(ClientState);
    }
}

void Update(double deltaTime)
{
    Machine.Update(deltaTime);
}

void Resize(Vec2i newSize)
{
    Machine.Resize();
}
