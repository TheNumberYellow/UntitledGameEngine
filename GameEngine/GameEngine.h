#pragma once

#include "Math/Math.h"

#include "StringUtils.h"

#include "Platform/EnginePlatform.h"
#include "Platform/RendererPlatform.h"
#include "Platform/NetworkPlatform.h"

#include "Modules/ModuleManager.h"

#include "Behaviour/Behaviour.h"

#include "ControlInputs.h"

#include "MeshGenerator.h"
#include "FileLoader.h"

// These functions are to be defined in client code
extern void Initialize(ModuleManager& modules);
extern void Update(ModuleManager& modules);
extern void Resize(ModuleManager& modules, Vec2i newSize);