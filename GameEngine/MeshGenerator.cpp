#include "MeshGenerator.h"

#include "Platform\RendererPlatform.h"

StaticMesh_ID MeshGenerator::GenCube(Renderer& renderer)
{
    //std::vector<float> vertices;
    //std::vector<ElementIndex> indices;

    //vertices =
    //{
    //    Vertex(Vec3f(-0.5, -0.5, -0.5), Vec3f(), Vec4f(1.0, 0.0, 0.0, 1.0), Vec2f(0.0, 0.0)),
    //    Vertex(Vec3f(-0.5, 0.5, -0.5), Vec3f(), Vec4f(0.0, 1.0, 0.0, 1.0), Vec2f(0.0, 1.0)),
    //    Vertex(Vec3f(0.5, 0.5, -0.5), Vec3f(), Vec4f(0.0, 0.0, 1.0, 1.0), Vec2f(1.0, 1.0)),
    //    Vertex(Vec3f(0.5, -0.5, -0.5), Vec3f(), Vec4f(1.0, 1.0, 0.0, 1.0), Vec2f(1.0, 0.0)),

    //    Vertex(Vec3f(-0.5, -0.5, 0.5), Vec3f(), Vec4f(0.5, 0.5, 0.5, 1.0), Vec2f(0.0, 0.0)),
    //    Vertex(Vec3f(-0.5, 0.5, 0.5), Vec3f(), Vec4f(1.0, 0.0, 1.0, 1.0), Vec2f(0.0, 1.0)),
    //    Vertex(Vec3f(0.5, 0.5, 0.5), Vec3f(), Vec4f(0.2f, 0.8f, 0.4f, 1.0f), Vec2f(1.0, 1.0)),
    //    Vertex(Vec3f(0.5, -0.5, 0.5), Vec3f(), Vec4f(0.4f, 0.2f, 0.8f, 1.0f), Vec2f(1.0, 0.0)),
    //};

    //indices =
    //{
    //    0, 2, 1, 0, 3, 2,
    //    4, 5, 6, 4, 6, 7,

    //    0, 4, 7, 0, 7, 3,
    //    1, 2, 6, 1, 6, 5,

    //    1, 5, 4, 1, 4, 0,
    //    3, 7, 6, 3, 6, 2
    //};

    //return renderer.LoadMesh(vertices, indices);
    return 0;
}

StaticMesh_ID MeshGenerator::GenTriangularPrism(Renderer& renderer)
{
    return 0;
}

StaticMesh_ID MeshGenerator::GenCylinder(Renderer& renderer)
{
    return 0;
}
