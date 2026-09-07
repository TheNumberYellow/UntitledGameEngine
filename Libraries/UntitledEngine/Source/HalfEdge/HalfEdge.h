#pragma once
#include "Interfaces/EditorClickable_i.h"
#include "Math/Math.h"


#include "States/Editor/CursorState.h"

class Model;
struct HalfEdgeMesh;
struct HotspotTexture;

namespace he
{
    struct Face;
    struct HalfEdge;
    struct Vertex;

    // Helper structs for half edge operation
    struct PolygonFace
    {
        std::vector<Vec3f> Vertices;
        
        // Keep pointer to half edge face this polygon came from to apply material data back to half edge mesh after processing
        he::Face* OriginalFace = nullptr;
    };

    struct FaceIsland
    {
        std::vector<he::Face*> Faces;
    };



    // Helper functions for half edge operations
    void ConvertHalfEdgeMeshToPolys(HalfEdgeMesh& mesh, std::vector<PolygonFace>& outPolys);
    void SplitPolygonByPlane(PolygonFace& poly, Plane inPlane, PolygonFace& outFrontPoly, PolygonFace& outBackPoly, std::vector<Vec3f>* intersectionPoints = nullptr);
    void AddPolygonToHalfEdgeMesh(HalfEdgeMesh& mesh, PolygonFace& poly, Material inMaterial);
    PolygonFace BuildCapPolygonForSlice(std::vector<Vec3f>& intersectionPoints, Plane slicePlane);
    bool IsPolygonConvex(PolygonFace& poly);
    
    std::vector<FaceIsland> BuildFaceIslands(HalfEdgeMesh& mesh);

    // Returns a vector of polygons that are in the same island as the input face, with faces rotated to be coplanar with the input face
    std::vector<PolygonFace> GetFlattenedAttachedFaces(HalfEdgeMesh& mesh, he::Face* face);

    // Helper functions for debug drawing half edge meshes
    void DebugDrawSelectedHalfEdgeMeshFace(HalfEdgeMesh& mesh, Face* face, Vec3f colour);
    void DebugDrawSelectedHalfEdgeMeshEdge(HalfEdgeMesh& mesh, HalfEdge* edge, Vec3f colour);
    void DebugDrawSelectedHalfEdgeMeshVertex(HalfEdgeMesh& mesh, he::Vertex* vert, Vec3f colour);

    using Index = int64_t;

    struct HalfEdgeMesh;

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

        bool isSeam = false;

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
        //TODO: store normals/smoothing data
    public:
        float textureNudgeU = 0.0f;
        float textureNudgeV = 0.0f;

        float textureScaleU = 1.0f;
        float textureScaleV = 1.0f;

        float textureRot = 0.0f;

        bool useUVOverride = false;
        std::vector<Vec2f> uvOverrides;

        HotspotTexture* appliedHotspotTexture = nullptr;

        // Temp debugging
        Vec3f m_bestRectStartPos;
        Vec2f m_bestRectSize;
        Vec3f m_bestRectRight;
        Vec3f m_bestRectUp;

        Material material;
        
        bool flipFace = false; // Whether this face should be rendered with reversed winding order, used for faces that have been flipped by the user to avoid having to recalculate halfedge connectivity

        void ApplyHotspotTexture(HotspotTexture& inHotspotTexture);
        Vec3f GetNormal();

        HalfEdge* halfEdge = nullptr;
    };

    class SelectedHalfEdgeMesh : public ISelectedObject
    {
    public:
        SelectedHalfEdgeMesh(HalfEdgeMesh* inMeshPtr);

        virtual void Draw() override;
        virtual void Update() override;

        virtual bool DrawInspectorPanel() override;
        virtual Transform* GetTransform() override;
        virtual void DeleteObject() override;

        virtual void ApplyMaterial(Material& inMaterial) override;
        virtual void ApplyHotspotTexture(HotspotTexture& inHotspotTexture) override;

    private:

        virtual bool IsEqual(const ISelectedObject& other) const override;


        Transform m_Transform;

        std::vector<Mat4x4f> m_VertTransOffsets;

        bool tempShowingIslands = false;


    public:
        HalfEdgeMesh* m_HalfEdgeMesh;
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

        he::Vertex* m_VertPtr = nullptr;
        Transform m_Transform;

        bool m_Dirty = false;
        
    public:
        HalfEdgeMesh* m_HalfEdgeMesh;
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
        virtual void ApplyHotspotTexture(HotspotTexture& inHotspotTexture) override;
        
    private:

        virtual bool IsEqual(const ISelectedObject& other) const override;

        bool isUserExtruding = false;

        he::Face* m_FacePtr = nullptr;
        Transform m_Transform;
        
        bool shouldDebugDrawHotspotRect = false;
        std::vector<Mat4x4f> m_VertTransOffsets;
        
        bool tempShowingFlattenedAttachedFaces = false;

    public:
        HalfEdgeMesh* m_HalfEdgeMesh;
    };

    class SelectedHalfEdgeEdge : public ISelectedObject
    {
    public:

        SelectedHalfEdgeEdge(HalfEdgeMesh* inMeshPtr, he::HalfEdge* inEdgePtr);

        virtual void Draw() override;
        virtual void Update() override;

        virtual bool DrawInspectorPanel() override;
        virtual Transform* GetTransform() override;
        virtual void DeleteObject() override;

    private:

        virtual bool IsEqual(const ISelectedObject& other) const override;


        he::HalfEdge* m_EdgePtr = nullptr;
        Transform m_Transform;

        Mat4x4f edgeOffsetA;
        Mat4x4f edgeOffsetB;

        HalfEdgeMesh* m_HalfEdgeMesh;

    };

    struct HalfEdgeMesh : public IEditorClickable
    {
        void MakeQuad();
        void MakeAABB(AABB inAABB, Material inMaterial);

        void MakeCylinder(Vec3f baseCenter, float radius, float height, int numSegments, Material inMaterial);

        void SubDivide();

        void SubDivideFace(Face* inFace);

        void DeleteFace(Face* inFace);

        void FlipFace(Face* inFace);

        void ExtrudeFace(he::Face* inFace);

        void SplitEdge(HalfEdge* inEdge);

        bool Slice(Plane inPlane, HalfEdgeMesh* outFrontMesh = nullptr, HalfEdgeMesh* outBackMesh = nullptr, bool addCaps = true);
        void SliceFaces(Plane inPlane);

        void EditorDraw();

        void Clear();

        std::vector<Vertex*> m_Verts;
        std::vector<Face*> m_Faces;
        std::vector<HalfEdge*> m_HalfEdges;

        std::vector<Model> m_RepModels;

        bool m_RepModelsNeedUpdate = true;

        bool m_BlendIslands = true;

        // Inherited via IEditorClickable
        //RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;

        RayCastHit RayCast(Ray ray);
        Intersection SphereIntersect(Sphere sphere);

        RayCastHit ClickCastFaces(Ray mouseRay, ISelectedObject*& outSelectedObject);
        RayCastHit ClickCastEdges(Ray mouseRay, ISelectedObject*& outSelectedObject);
        RayCastHit ClickCastVerts(Ray mouseRay, ISelectedObject*& outSelectedObject);

        Face* RayCastFaces(Ray mouseRay, Vec3f& outHitPoint, float& outHitDistance);
        HalfEdge* RayCastEdges(Ray mouseRay, Vec3f& outHitPoint, float& outHitDistance);
        Vertex* RayCastVerts(Ray mouseRay, Vec3f& outHitPoint, float& outHitDistance);

        virtual RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;

    };

}