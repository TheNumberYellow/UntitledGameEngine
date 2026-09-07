#include "Decal.h"

#include "Modules/GraphicsModule.h"
#include "Modules/UIModule.h"

#include "Scene/Scene.h"


SelectedDecal::SelectedDecal(Decal* inDecalPtr)
{
    DecalPtr = inDecalPtr;

    Trans.SetTransformMatrix(DecalPtr->orientation);
}

void SelectedDecal::Draw()
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    // Draw the actual projector box of the decal
    AABB DecalProjectorAABB = AABB(Vec3f(-0.5f, -0.5f, -0.5f), Vec3f(0.5f, 0.5f, 0.5f));

    Graphics->DebugDrawAABB(DecalProjectorAABB, Vec3f(1.0f, 0.0f, 1.0f), Trans.GetTransformMatrix());

    // Draw arrow to show the direction of the decal projection
    Vec3f forwardVector = Vec3f(-1.0f, 0.0f, 0.0f) * Trans.GetRotation();
    Vec3f arrowStart = Trans.GetPosition() - (forwardVector * Trans.GetScale() * 0.5f);
    Vec3f arrowEnd = Trans.GetPosition();

    Graphics->DebugDrawArrow(arrowStart, arrowEnd, Vec3f(0.9f, 0.9f, 1.0f));

    InputModule* Input = InputModule::Get();

    if (Input->IsKeyDown(Key::P))
    {
        std::vector<Plane> planes = DecalPtr->GetProjectionCubePlanes();
        for (const Plane& plane : planes)
        {
            Graphics->DebugDrawPlane(plane, 1.0f, Vec3f(1.0f, 0.0f, 0.0f));
        }
    }

}

void SelectedDecal::Update()
{
    DecalPtr->orientation = Trans.GetTransformMatrix();
}

bool SelectedDecal::DrawInspectorPanel()
{
    return false;
}

Transform* SelectedDecal::GetTransform()
{
    return &Trans;
}

void SelectedDecal::DeleteObject()
{
    ScenePtr->DeleteDecal(DecalPtr);
}

bool SelectedDecal::IsEqual(const ISelectedObject& Other) const
{
    const SelectedDecal* OtherDecal = dynamic_cast<const SelectedDecal*>(&Other);
    if (OtherDecal == nullptr)
    {
        return false;
    }
    return DecalPtr == OtherDecal->DecalPtr;
}

Decal::Decal()
{
    // Scale the orientation to 2
    orientation.m_Rows[0] = Vec4f(2.0f, 0.0f, 0.0f, 0.0f);
    orientation.m_Rows[1] = Vec4f(0.0f, 2.0f, 0.0f, 0.0f);
    orientation.m_Rows[2] = Vec4f(0.0f, 0.0f, 2.0f, 0.0f);
    orientation.m_Rows[3] = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
}

std::vector<Plane> Decal::GetProjectionCubePlanes()
{
    // Get 6 planes of the cube
    std::vector<Plane> planes;

    Vec3f position = Vec3f(orientation.m_Rows[3].x, orientation.m_Rows[3].y, orientation.m_Rows[3].z);

    Vec3f right = Vec3f(orientation.m_Rows[0].x, orientation.m_Rows[0].y, orientation.m_Rows[0].z);

    Vec3f up = Vec3f(orientation.m_Rows[1].x, orientation.m_Rows[1].y, orientation.m_Rows[1].z);

    Vec3f forward = Vec3f(orientation.m_Rows[2].x, orientation.m_Rows[2].y, orientation.m_Rows[2].z);

    planes.push_back(Plane(position + right * 0.5f, right));
    planes.push_back(Plane(position - right * 0.5f, -right));

    planes.push_back(Plane(position + up * 0.5f, up));
    planes.push_back(Plane(position - up * 0.5f, -up));

    planes.push_back(Plane(position + forward * 0.5f, forward));
    planes.push_back(Plane(position - forward * 0.5f, -forward));

    return planes;
}

RayCastHit Decal::ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject)
{
    CollisionModule* collisions = CollisionModule::Get();

    Vec3f position = Vec3f(orientation.m_Rows[3].x, orientation.m_Rows[3].y, orientation.m_Rows[3].z);

    AABB DecalAABB = AABB(position - Vec3f(0.35f, 0.35f, 0.35f), position + Vec3f(0.35f, 0.35f, 0.35f));

    RayCastHit result = collisions->RayCast(mouseRay, DecalAABB);

    if (result.hit)
    {
        outSelectedObject = new SelectedDecal(this);
    }
    return result;
}
