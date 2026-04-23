#include "GUID.hpp"

uGUIDGenerator::uGUIDGenerator()
{
    currentuGUID = 0;
}

uGUIDGenerator::uGUIDGenerator(uGUID initialuGUID)
{
    currentuGUID = initialuGUID;
}

uGUID uGUIDGenerator::Generate()
{
    std::lock_guard<std::mutex> lock(guidMtx);
    if (freeduGUIDs.empty())
    {
        return currentuGUID++;
    }
    else
    {
        uGUID ret = freeduGUIDs.top();
        freeduGUIDs.pop();
        return ret;
    }
}

void uGUIDGenerator::FreeID(uGUID inID)
{
    std::lock_guard<std::mutex> lock(guidMtx);
    freeduGUIDs.push(inID);
}

void uGUIDGenerator::Reset()
{
    std::lock_guard<std::mutex> lock(guidMtx);

    while (!freeduGUIDs.empty())
    {
        freeduGUIDs.pop();
    }

    currentuGUID = 0;
}
