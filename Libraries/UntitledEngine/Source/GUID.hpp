#pragma once

#include <mutex>
#include <stack>

#define GUID unsigned int

class GUIDGenerator
{
public:
    GUIDGenerator();
    GUIDGenerator(GUID initialGUID);

    GUID Generate();
    void FreeID(GUID inID);

    void Reset();
private:
    GUID currentGUID;

    std::stack<GUID> freedGUIDs;

    std::mutex guidMtx;
};