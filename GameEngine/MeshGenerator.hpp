#pragma once

// All shapes are generated at unit scale with no rotation at the origin 
// Mesh functions for scaling/rotating/translating can be done with mesh functions

#include "RendererPlatform.hpp"

class MeshGenerator
{
public:

	static Mesh_ID GenCube(Renderer& renderer);
	static Mesh_ID GenTriangularPrism(Renderer& renderer);
	static Mesh_ID GenCylinder(Renderer& renderer);
};

