#pragma once

#include "Asset/AssetRegistry.h"
#include "Material.h"
#include "Interfaces/EditorClickable_i.h"

class Model;
class Scene;

struct StaticMeshRenderCommand
{
    StaticMesh_ID m_Mesh;
    Material m_Material;
    Mat4x4f m_TransMat;
};

enum class ModelType
{
    BLOCK,
    PLANE,
    MODEL
};

struct TexturedMesh
{
    TexturedMesh() {}

    TexturedMesh(StaticMesh mesh, Material material)
        : m_Mesh(mesh)
        , m_Material(material)
    {}

    StaticMesh m_Mesh;

    Material m_Material;
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

class Model : public IEditorClickable
{
public:
    Model()
        : m_Transform()
    {}
    Model(TexturedMesh texturedMesh)
        : m_Transform()
    {
        m_TexturedMeshes.push_back(texturedMesh);
    }

    Transform& GetTransform()
    {
        return m_Transform;
    }

    void SetMaterial(Material material)
    {
        m_TexturedMeshes[0].m_Material = material;
    }

    virtual RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;

    std::vector<TexturedMesh> m_TexturedMeshes;

    std::string m_Name = "";

    ModelType Type = ModelType::MODEL;
private:
    Transform m_Transform;
};