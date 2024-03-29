#pragma once
#include "Behaviour/Behaviour.h"

class DungeonCornerstone : public Behaviour
{
public:
    DEFINE_BEHAVIOUR(DungeonCornerstone);

    virtual void Initialize(Scene* Scene) override;
    virtual void Update(Scene* Scene, float DeltaTime) override;

private:

    void AddBlock(AABB block, Material mat);
    void AddHallway(AABB hallway, bool xDirection);

    Scene* ContainingScene = nullptr;

    Material GroundMat;
    Material CeilingMat;
    AABB CornerstoneAABB;

    float FloorHeight;
    const float HallWidth = 8.0f;
    const float HalfHallWidth = HallWidth / 2.0f;
    const float HallHeight = 6.0f;
};

