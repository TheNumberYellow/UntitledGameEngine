#include "GUID.hpp"

GUIDGenerator::GUIDGenerator()
{
    currentGUID = 0;
}

GUIDGenerator::GUIDGenerator(GUID initialGUID)
{
    currentGUID = initialGUID;
}

GUID GUIDGenerator::Generate()
{
    std::lock_guard<std::mutex> lock(guidMtx);
    if (freedGUIDs.empty())
    {
        return currentGUID++;
    }
    else
    {
        GUID ret = freedGUIDs.top();
        freedGUIDs.pop();
        return ret;
    }
}

void GUIDGenerator::FreeID(GUID inID)
{
    std::lock_guard<std::mutex> lock(guidMtx);
    freedGUIDs.push(inID);
}

void GUIDGenerator::Reset()
{
    std::lock_guard<std::mutex> lock(guidMtx);

    while (!freedGUIDs.empty())
    {
        freedGUIDs.pop();
    }

    currentGUID = 0;
}
