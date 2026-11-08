#pragma once

#include <mutex>
#include <stack>

#define uGUID long long

class uGUIDGenerator
{
public:
    uGUIDGenerator();
    uGUIDGenerator(uGUID initialuGUID);

    uGUID Generate();
    void FreeID(uGUID inID);

    void Reset();
private:
    uGUID currentuGUID;

    std::stack<uGUID> freeduGUIDs;

    std::mutex guidMtx;
};