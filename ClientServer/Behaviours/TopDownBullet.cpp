#include "TopDownBullet.h"

REGISTER_BEHAVIOUR(TopDownBullet);

void TopDownBullet::Update(Scene* Scene, double DeltaTime)
{
    if (!Initialized)
    {
        PointLight NewLight;
        NewLight.colour = Vec3f(191.f / 255.f, 64.f / 255.f, 191.f / 255.f);

        Light = Scene->AddPointLight(NewLight);

        Initialized = true;
    }

    m_Model->GetTransform().Move((float)DeltaTime * (float)BulletSpeed * Direction);

    Light->position = m_Model->GetTransform().GetPosition();

    BulletLifeTime -= DeltaTime;

    if (BulletLifeTime <= 0.0f)
    {
        Scene->DeletePointLight(Light);
        Scene->DeleteModel(m_Model);
    }

    std::vector<Model*> Ghosts = Scene->GetModelsByTag("Ghost");

    //std::string DebugOutput = "Num ghosts: " + std::to_string(Ghosts.size());

    //Engine::DEBUGPrint(DebugOutput);

    for (auto& G : Ghosts)
    {
        Vec3f GhostPos = G->GetTransform().GetPosition();
        Vec3f MyPos = m_Model->GetTransform().GetPosition();

        Vec2f GhostPos2D = Vec2f(GhostPos.x, GhostPos.y);
        Vec2f MyPos2D = Vec2f(MyPos.x, MyPos.y);

        float Dist = Math::magnitude(GhostPos2D - MyPos2D);

        if (Dist < 1.0f)
        {
            Scene->DeleteModel(G);
            //Scene->DeleteModel(m_Model);
            //return;
        }
    }
}
