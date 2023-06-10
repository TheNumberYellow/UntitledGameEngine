#include "SpinBehaviour.h"

REGISTER_BEHAVIOUR(SpinBehaviour);

SpinBehaviour::SpinBehaviour()
{
}

void SpinBehaviour::Update(float DeltaTime)
{
    Engine::DEBUGPrint("SPIN");
}
