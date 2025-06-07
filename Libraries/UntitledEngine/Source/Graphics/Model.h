#pragma once

#include "Asset/AssetRegistry.h"
#include "Components/Component.h"
#include "Material.h"
#include "Interfaces/EditorClickable_i.h"
#include "RenderableInterface.h"

class Model;
class Scene;

struct StaticMeshRenderCommand
{
    StaticMesh_ID m_Mesh;
    Material m_Material;
    Mat4x4f m_TransMat;
    bool m_CastShadows = true;
};

enum class ModelType
{
    BLOCK,
    PLANE,
    MODEL
};

class SelectedModel : public ISelectedObject
{
public:
    SelectedModel(Model* InModel);

    virtual void Draw() override;

    virtual bool DrawInspectorPanel() override;

    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

    virtual void ApplyMaterial(Material& inMaterial) override;


private:

    virtual bool IsEqual(const ISelectedObject& Other) const override;

    Model* ModelPtr;
};

class Model : 
    public IEditorClickable, 
    public Component
{
public:
    Model()
        : m_Transform()
    {}
    Model(StaticMesh inStaticMesh, Material inMaterial)
        : m_StaticMesh(inStaticMesh)
        , m_Material(inMaterial)
    {
    }

    Transform& GetTransform()
    {
        return m_Transform;
    }

    void SetMaterial(Material inMaterial)
    {
        m_Material = inMaterial;
    }

    virtual RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;

    Material m_Material;
    StaticMesh m_StaticMesh;

    std::string m_Name = "";

    ModelType Type = ModelType::MODEL;
private:
    Transform m_Transform;
};