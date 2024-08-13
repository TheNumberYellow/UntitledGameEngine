#include "Ghost.h"

#include "TopDownPlayer.h"

REGISTER_BEHAVIOUR(Ghost);

void Ghost::Update(Scene* Scene, double DeltaTime)
{
    if (!Started)
    {
        //Target = Scene->GetModelByTag("Player");

        Started = true;
    }

    if (Target)
    {
        Vec3f GhostToTarget = Target->GetTransform().GetPosition() - m_Model->GetTransform().GetPosition();

        Vec2f InputDir2D = Vec2f(GhostToTarget.x, GhostToTarget.y);

        if (Math::magnitude(InputDir2D) < 0.25f)
        {
            TopDownPlayer* PlayerBehaviour = static_cast<TopDownPlayer*>(BehaviourRegistry::Get()->GetBehaviourAttachedToEntity(Target));
            if (PlayerBehaviour)
            {
                PlayerBehaviour->Hurt();
            }

            Scene->DeleteModel(m_Model);

            return;
        }

        GhostToTarget = Math::normalize(GhostToTarget);

        Vec2f Left2D = Vec2f(1.0f, 0.0f);

        float dot = Math::dot(InputDir2D, Left2D);
        float det = (InputDir2D.x * Left2D.y) - (InputDir2D.y * Left2D.x);

        float angle = atan2(det, dot);

        Quaternion r = Quaternion(Vec3f(0.0f, 0.0f, 1.0f), -angle);
        m_Model->GetTransform().SetRotation(r);

        Vec3f GhostToTargetHorizontal = Math::normalize(Vec3f(GhostToTarget.x, GhostToTarget.y, 0.0f));

        m_Model->GetTransform().Move(GhostToTargetHorizontal * GhostSpeed * (float)DeltaTime);


    }
}
