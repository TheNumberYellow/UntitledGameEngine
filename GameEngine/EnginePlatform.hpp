#pragma once

#include "Math.hpp"

#include <string>

namespace Engine
{
	// These functions are defined in platform-specific engine code
	extern float GetElapsedTime();

	extern Vec2i GetScreenSize();
	extern Vec2i GetMousePosition();

	extern void DEBUGPrint(std::string string);

	extern void LockCursor();
	extern void UnlockCursor();

	extern void StopGame();
}
