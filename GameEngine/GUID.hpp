#pragma once

#include <stack>

#define GUID unsigned int

class GUIDGen
{

public:
    static GUID Generate();
    static void FreeID(GUID inID);
private:
    static GUID currentGUID;

    static std::stack<GUID> freedGUIDs;
};