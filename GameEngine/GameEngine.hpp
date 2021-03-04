#pragma once

#include "Math.hpp"

#include "StringUtils.hpp"

#include "EnginePlatform.hpp"
#include "RendererPlatform.hpp"

#include "ControlInputs.hpp"

#include "Scene.h"
#include "MeshGenerator.hpp"
#include "FileLoader.h"

// These functions are to be defined in client code
extern void Initialize(Renderer& renderer);
extern void Update(Renderer& renderer, ControlInputs& inputs);