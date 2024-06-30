#include "DungeonCornerstone.h"

REGISTER_BEHAVIOUR(DungeonCornerstone);

void DungeonCornerstone::Initialize(Scene* Scene)
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();

    ContainingScene = Scene;

    GroundMat = m_Model->m_TexturedMeshes[0].m_Material;


    CornerstoneAABB = Collisions->GetCollisionMeshFromMesh(m_Model->m_TexturedMeshes[0].m_Mesh)->boundingBox;
    CornerstoneAABB.min += m_Model->GetTransform().GetPosition();
    CornerstoneAABB.max += m_Model->GetTransform().GetPosition();

    float BottomFloor = CornerstoneAABB.min.z;
    float TopFloor = CornerstoneAABB.max.z;

    FloorHeight = TopFloor - BottomFloor;

    Vec2f FloorCenter = Vec2f(CornerstoneAABB.Center().x, CornerstoneAABB.Center().y);

    Vec2f EastStart = FloorCenter + (Vec2f(CornerstoneAABB.XSize() / 2.0f, 0.0f));

    AABB EastHallway = AABB(Vec3f(EastStart.x, EastStart.y - (HalfHallWidth), TopFloor), Vec3f(EastStart.x + 64.0f, EastStart.y + HalfHallWidth, HallHeight));

    AddHallway(EastHallway, true);
    //AABB EastHallwayCeiling = AABB(EastHallwayFloor.min + Vec3f(0.0f, 0.0f, HallHeight), EastHallwayFloor.max + Vec3f(0.0f, 0.0f, HallHeight));

    //AddBlock(EastHallwayFloor, GroundMat);
    //AddBlock(EastHallwayCeiling, GroundMat);

    //Vec3f NewBlockPos = m_Model->GetTransform().GetPosition();

    //NewBlockPos.x += 4.0f;
}

void DungeonCornerstone::Update(Scene* Scene, float DeltaTime)
{
    //GraphicsModule* Graphics = GraphicsModule::Get();

    //Graphics->DebugDrawAABB(CornerstoneAABB);

}

void DungeonCornerstone::AddBlock(AABB block, Material mat)
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    ContainingScene->AddModel(new Model(Graphics->CreateBoxModel(block, mat)));
}

void DungeonCornerstone::AddHallway(AABB hallway, bool xDirection)
{
    PointLight Light;
    Light.position = hallway.Center();
    Light.colour = Vec3f(255.f / 255.f, 191.f / 255.f, 0.f / 255.f);
    ContainingScene->AddPointLight(Light);

    Vec3f MinFloor = Vec3f(hallway.min.x, hallway.min.y, hallway.min.z - FloorHeight);
    Vec3f MaxFloor = Vec3f(hallway.max.x, hallway.max.y, hallway.min.z);
    AABB Floor = AABB(MinFloor, MaxFloor);

    Vec3f MinCeil = Vec3f(hallway.min.x, hallway.min.y, hallway.max.z);
    Vec3f MaxCeil = Vec3f(hallway.max.x, hallway.max.y, hallway.max.z + FloorHeight);
    AABB Ceil = AABB(MinCeil, MaxCeil);

    AABB LeftWall;
    AABB RightWall;

    if (xDirection)
    {
        Vec3f MinWall = Vec3f(hallway.min.x, hallway.max.y, hallway.min.z);
        Vec3f MaxWall = Vec3f(hallway.max.x, hallway.max.y + FloorHeight, hallway.max.z);
    
        LeftWall = AABB(MinWall, MaxWall);

        MinWall -= Vec3f(0.0f, hallway.YSize() + FloorHeight, 0.0f);
        MaxWall -= Vec3f(0.0f, hallway.YSize() + FloorHeight, 0.0f);

        RightWall = AABB(MinWall, MaxWall);
    }
    else
    {

    }

    AddBlock(Floor, GroundMat);
    AddBlock(Ceil, GroundMat);
    AddBlock(LeftWall, GroundMat);
    AddBlock(RightWall, GroundMat);
}
