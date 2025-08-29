#pragma once

#include "..\Math\Math.h"

#include <string>

namespace Engine
{
    // These functions are defined in platform-specific engine code
    extern float GetElapsedTime();

    extern Vec2i GetWindowSize();
    extern Vec2i GetClientAreaSize();

    extern Vec2i GetMousePosition();
    extern void SetMousePosition(Vec2i pos);
    
    extern bool GetMouseDown(int button = 0);

    extern void DEBUGPrint(std::string string);
    extern void Alert(std::string message);
    extern void Error(std::string errorMessage);
    extern void FatalError(std::string errorMessage);

    extern void SetCursorCenter(Vec2i center);

    extern void LockCursor();
    extern void UnlockCursor();

    extern void HideCursor();
    extern void ShowCursor();

    extern void StopGame();

    extern bool IsGameStopped();

    extern bool IsWindowFocused();

    extern bool FileOpenDialog(std::string& OutFileString);
    extern bool FileSaveDialog(std::string& OutFileString, std::string DialogTitle, std::string FileTypeName, std::string FileTypeExt);

    extern void SetWindowTitleText(std::string Text);

    extern void CreateNewWindow();

    extern void RunCommand(std::string Command);

}