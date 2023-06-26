#pragma once

// All shapes are generated at unit scale with no rotation at the origin 
// Mesh functions for scaling/rotating/translating can be done with mesh functions

#include "Platform\RendererPlatform.h"

class MeshGenerator
{
public:

    static StaticMesh_ID GenCube(Renderer& renderer);
    static StaticMesh_ID GenTriangularPrism(Renderer& renderer);
    static StaticMesh_ID GenCylinder(Renderer& renderer);
};

