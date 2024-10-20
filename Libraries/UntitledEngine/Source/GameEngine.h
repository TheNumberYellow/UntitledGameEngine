#pragma once

#include "Math/Math.h"

#include "StringUtils.h"

#include "Platform/EnginePlatform.h"
#include "Platform/RendererPlatform.h"
#include "Platform/NetworkPlatform.h"

#include "Modules/ModuleManager.h"

#include "Behaviour/Behaviour.h"

#include "Scene.h"

#include "MeshGenerator.h"
#include "FileLoader.h"

using ArgsList = std::string;

// These functions are to be defined in client code
extern void Initialize(ArgsList args);
extern void Update(double deltaTime);
extern void Resize(Vec2i newSize);