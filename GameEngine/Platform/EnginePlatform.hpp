#pragma once

#include "..\Math\Math.hpp"

#include <string>

namespace Engine
{
    // These functions are defined in platform-specific engine code
    extern float GetElapsedTime();

    extern Vec2i GetWindowSize();
    extern Vec2i GetClientAreaSize();
    extern Vec2i GetMousePosition();
    extern bool GetMouseDown(int button = 0);

    extern void DEBUGPrint(std::string string);
    extern void FatalError(std::string errorMessage);

    extern void SetCursorCenter(Vec2i center);

    extern void LockCursor();
    extern void UnlockCursor();

    extern void HideCursor();
    extern void ShowCursor();

    extern void StopGame();
}