#include "Model.h"

#include "Behaviour/Behaviour.h"
#include "Modules/CollisionModule.h"
#include "Modules/UIModule.h"
#include "Scene.h"

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
    UIModule* UI = UIModule::Get();

    float oldX = ModelPtr->GetTransform().GetPosition().x;
    float oldY = ModelPtr->GetTransform().GetPosition().y;
    float oldZ = ModelPtr->GetTransform().GetPosition().z;

    float newX = oldX;
    float newY = oldY;
    float newZ = oldZ;

    UI->FloatSlider("X", Vec2f(400.0f, 20.0f), newX, -10.0f, 10.0f);
    UI->FloatSlider("Y", Vec2f(400.0f, 20.0f), newY, -10.0f, 10.0f);
    UI->FloatSlider("Z", Vec2f(400.0f, 20.0f), newZ, -10.0f, 10.0f);

    if (newX != oldX || newY != oldY || newZ != oldZ)
    {
        ModelPtr->GetTransform().SetPosition(Vec3f(newX, newY, newZ));
        return true;
    }

    UI->NewLine();

    BehaviourRegistry::Get()->DrawEntityInspectorPanel(ModelPtr);

    return false;
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
