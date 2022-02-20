#pragma once

#include "Math\Math.hpp"

struct KeysDown
{

    bool esc = false;
    bool w = false;
    bool a = false;
    bool s = false;
    bool d = false;
    bool q = false;
    bool e = false;

    bool zero = false;
    bool one = false;
    bool two = false;
    bool three = false;
    bool four = false;
    bool five = false;
    bool six = false;
    bool seven = false;
    bool eight = false;
    bool nine = false;
    
    bool space = false;
    bool alt = false;
    bool tab = false;
    bool shift = false;
    bool ctrl = false;
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};

struct Mouse
{
    bool leftMouseButton = false;
    bool rightMouseButton = false;
};

struct ControlInputs
{
    // Change in mouse position since last iteration
    Vec2i DeltaMouse;

    Vec2i LatestMouse;

    KeysDown keysDown;
    Mouse mouse;
};