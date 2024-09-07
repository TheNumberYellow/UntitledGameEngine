#include "GUID.hpp"

GUID GUIDGen::currentGUID = 0;
std::stack<GUID> GUIDGen::freedGUIDs;

GUID GUIDGen::Generate()
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

void GUIDGen::FreeID(GUID inID)
{
    freedGUIDs.push(inID);
}
