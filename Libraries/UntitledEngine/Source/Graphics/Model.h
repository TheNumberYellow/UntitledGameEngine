#pragma once

#include "Asset/AssetRegistry.h"
#include "Components/Component.h"
#include "Material.h"
#include "Interfaces/EditorClickable_i.h"
#include "RenderableInterface.h"
#include "Scene/SceneObject.h"

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

    //Temporarily public
public:
    Model* ModelPtr;
};

class Model : 
    public IEditorClickable 
    , public SceneObject
{
public:
    Model()
        : SceneObject(nullptr)
        , m_Transform()
    {}

    Model(Scene* inScene)
        : SceneObject(inScene)
        , m_Transform()
    {}
    Model(Scene* inScene, StaticMesh inStaticMesh, Material inMaterial)
        : SceneObject(inScene)
        , m_Transform()
        , m_StaticMesh(inStaticMesh)
        , m_Material(inMaterial)
    {
    }
    Model(Scene* inScene, StaticMesh inStaticMesh, Material inMaterial, Transform inTransform)
        : SceneObject(inScene)
        , m_Transform(inTransform)
        , m_StaticMesh(inStaticMesh)
        , m_Material(inMaterial)
    {
    }


    bool DrawInspectorPanel() override;

    Transform& GetTransform()
    {
        return m_Transform;
    }

    void SetTransform(Transform inTransform)
    {
        m_Transform = inTransform;
    }

    void SetStaticMesh(StaticMesh inStaticMesh)
    {
        m_StaticMesh = inStaticMesh;
    }

    void SetMaterial(Material inMaterial)
    {
        m_Material = inMaterial;
    }

    void SetType(ModelType inType)
    {
        Type = inType;
    }


    virtual RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;

    Material m_Material;
    StaticMesh m_StaticMesh;

    std::string m_Name = "";

    ModelType Type = ModelType::MODEL;
private:
    Transform m_Transform;
};