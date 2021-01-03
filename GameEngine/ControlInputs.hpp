#pragma once

#include "Math.hpp"

struct KeysDown
{
	bool esc = false;
	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;
	bool space = false;
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