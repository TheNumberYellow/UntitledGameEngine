#pragma once
#include "Interfaces/EditorClickable_i.h"
#include "Math/Math.h"

#include "States/Editor/CursorState.h"

class Model;
struct HalfEdgeMesh;

namespace he
{
    using Index = int64_t;

    using VertIndex = Index;
    using FaceIndex = Index;
    using HalfEdgeIndex = Index;


    struct Vertex;
    struct Face;

    struct HalfEdge
    {
        VertIndex vert = -1;
        FaceIndex face = -1;
        HalfEdgeIndex twin = -1;
        HalfEdgeIndex next = -1;
    };

    struct Vertex
    {
        Vertex(Vec3f inVec) : vec(inVec) {};
        Vec3f vec;
        HalfEdgeIndex halfEdge = -1;
    };

    struct Face
    {
        //TODO: store material + maybe normals/smoothing data
        HalfEdgeIndex halfEdge = -1;
    };

    class SelectedHalfEdgeVertex : public ISelectedObject
    {
    public:

        SelectedHalfEdgeVertex(HalfEdgeMesh* inMeshPtr, VertIndex inVertIndex);

        virtual void Draw();

        virtual void Update();

        // Inherited via ISelectedObject
        bool DrawInspectorPanel() override;
        Transform* GetTransform() override;
        void DeleteObject() override;

    private:

        bool IsEqual(const ISelectedObject& other) const override;

        HalfEdgeMesh* m_HalfEdgeMesh;
        VertIndex m_VertIndex;
        Transform m_Transform;
    };

    struct HalfEdgeMesh : public IEditorClickable
    {
        void MakeQuad();

        void SubDivide();

        void Draw();

        void Clear();

        std::vector<Vertex> m_Verts;
        std::vector<Face> m_Faces;
        std::vector<HalfEdge> m_HalfEdges;

        Model* m_Model = nullptr;

        // Inherited via IEditorClickable
        RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;
    };


}