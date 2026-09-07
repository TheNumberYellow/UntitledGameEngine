#include "Model.h"

#include "Behaviour/Behaviour.h"
#include "Modules/CollisionModule.h"
#include "Modules/UIModule.h"
#include "Scene/Scene.h"

SelectedModel::SelectedModel(Model* InModel)
{
    ModelPtr = InModel;
}

void SelectedModel::Draw()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    CollisionModule* Collision = CollisionModule::Get();

    Graphics->DebugDrawAABB(Collision->GetCollisionMeshFromMesh(ModelPtr->m_StaticMesh)->boundingBox, c_SelectedBoxColour, ModelPtr->GetTransform().GetTransformMatrix());
}

bool SelectedModel::DrawInspectorPanel()
{
    return ModelPtr->DrawInspectorPanel();
}

Transform* SelectedModel::GetTransform()
{
    return &ModelPtr->GetTransform();
}

void SelectedModel::DeleteObject()
{
    ScenePtr->DeleteModel(ModelPtr);
}

void SelectedModel::ApplyMaterial(Material& inMaterial)
{
    ModelPtr->SetMaterial(inMaterial);
}

bool SelectedModel::IsEqual(const ISelectedObject& Other) const
{
    return ModelPtr == static_cast<const SelectedModel&>(Other).ModelPtr;
}

bool Model::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    bool ret = false;

    float oldX = GetTransform().GetPosition().x;
    float oldY = GetTransform().GetPosition().y;
    float oldZ = GetTransform().GetPosition().z;
    float newX = oldX;
    float newY = oldY;
    float newZ = oldZ;

    UI->FloatDragger("X", Vec2f(130.0f, 20.0f), newX);
    UI->FloatDragger("Y", Vec2f(130.0f, 20.0f), newY);
    UI->FloatDragger("Z", Vec2f(130.0f, 20.0f), newZ);

    if (newX != oldX || newY != oldY || newZ != oldZ)
    {
        GetTransform().SetPosition(Vec3f(newX, newY, newZ));
        ret = true;
    }

    UI->NewLine();

    BehaviourRegistry::Get()->DrawEntityInspectorPanel(this);

    return ret;
}

RayCastHit Model::ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject)
{
    CollisionModule* collisions = CollisionModule::Get();

    RayCastHit result = collisions->RayCast(mouseRay, *this);

    if (result.hit)
    {
        outSelectedObject = new SelectedModel(this);
    }

    return result;
}
