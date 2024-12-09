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
    freedGUIDs.push(inID);
}

void GUIDGenerator::Reset()
{
    while (!freedGUIDs.empty())
    {
        freedGUIDs.pop();
    }

    currentGUID = 0;
}
