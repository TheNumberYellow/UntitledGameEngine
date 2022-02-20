#pragma once

#include "..\Math\Vector.h"

class IResizeable
{
public:

	virtual void Resize(Vec2i newSize) = 0;
};