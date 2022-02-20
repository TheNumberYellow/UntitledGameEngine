#pragma once

#include "Math\Math.hpp"

#include "StringUtils.hpp"

#include "Platform\EnginePlatform.hpp"
#include "Platform\RendererPlatform.hpp"

#include "Modules\ModuleManager.h"

#include "ControlInputs.hpp"

#include "MeshGenerator.hpp"
#include "FileLoader.h"

// These functions are to be defined in client code
extern void Initialize(ModuleManager& modules);
extern void Update(ModuleManager& modules, ControlInputs& inputs);
extern void Resize(ModuleManager& modules, Vec2i newSize);