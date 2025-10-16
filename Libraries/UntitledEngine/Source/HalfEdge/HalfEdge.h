#pragma once
#include "Interfaces/EditorClickable_i.h"
#include "Math/Math.h"

#include "States/Editor/CursorState.h"

class Model;
struct HalfEdgeMesh;

namespace he
{
    using Index = int64_t;

    //using VertIndex = Index;
    //using FaceIndex = Index;
    //using HalfEdgeIndex = Index;


    struct Vertex;
    struct Face;

    struct HalfEdge
    {
        HalfEdge() {}

        HalfEdge(Vertex* inVert, Face* inFace, HalfEdge* inNext, HalfEdge* inTwin)
            : vert(inVert)
            , face(inFace)
            , next(inNext)
            , twin(inTwin)
        {}

        Vertex* vert = nullptr;
        Face* face = nullptr;
        HalfEdge* next = nullptr;
        HalfEdge* twin = nullptr;

        auto operator<=>(const HalfEdge& rhs) const = default;
    };

    struct Vertex
    {
        Vertex(Vec3f inVec) : vec(inVec) {};
        Vec3f vec;
        HalfEdge* halfEdge = nullptr;
    };

    struct Face
    {
        //TODO: store material + maybe normals/smoothing data
    public:
        float textureNudgeU = 0.0f;
        float textureNudgeV = 0.0f;

        float textureScaleU = 1.0f;
        float textureScaleV = 1.0f;

        float textureRot = 0.0f;

        Material* material = nullptr;

        HalfEdge* halfEdge = nullptr;
    };

    class SelectedHalfEdgeVertex : public ISelectedObject
    {
    public:

        SelectedHalfEdgeVertex(HalfEdgeMesh* inMeshPtr, he::Vertex* inVertptr);

        virtual void Draw() override;
        virtual void Update() override;

        virtual bool DrawInspectorPanel() override;
        virtual Transform* GetTransform() override;
        virtual void DeleteObject() override;

    private:

        virtual bool IsEqual(const ISelectedObject& other) const override;

        HalfEdgeMesh* m_HalfEdgeMesh;
        he::Vertex* m_VertPtr = nullptr;
        Transform m_Transform;

        bool m_Dirty = false;
    };

    class SelectedHalfEdgeFace : public ISelectedObject
    {
    public:

        SelectedHalfEdgeFace(HalfEdgeMesh* inMeshPtr, he::Face* inFacePtr);

        virtual void Draw() override;
        virtual void Update() override;

        virtual bool DrawInspectorPanel() override;
        virtual Transform* GetTransform() override;
        virtual void DeleteObject() override;

        virtual void ApplyMaterial(Material& inMaterial);
        
    private:

        virtual bool IsEqual(const ISelectedObject& other) const override;

        HalfEdgeMesh* m_HalfEdgeMesh;
        he::Face* m_FacePtr = nullptr;
        Transform m_Transform;
        
        std::vector<Mat4x4f> m_VertTransOffsets;
    };

    struct HalfEdgeMesh
    {
        void MakeQuad();
        void MakeAABB(AABB inAABB);

        void SubDivide();

        void SubDivideFace(Face* inFace);

        void ExtrudeFace(he::Face* inFace);

        void EditorDraw();

        void Clear();

        std::vector<Vertex*> m_Verts;
        std::vector<Face*> m_Faces;
        std::vector<HalfEdge*> m_HalfEdges;

        std::vector<Model> m_RepModels;


        // Inherited via IEditorClickable
        //RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;

        RayCastHit ClickCastFaces(Ray mouseRay, ISelectedObject*& outSelectedObject);
        RayCastHit ClickCastVerts(Ray mouseRay, ISelectedObject*& outSelectedObject);

    };

}