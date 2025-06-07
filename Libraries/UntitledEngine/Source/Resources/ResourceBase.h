#pragma once

#include <string>

using ResourceType = std::string;
using ResourceID = std::string;

class Resource
{

public:
    Resource(ResourceID uniqueID) : m_UniqueID(uniqueID) {}
    virtual ~Resource() {}

    virtual ResourceType GetType() = 0;

    virtual bool LoadResource() { return true; }

    ResourceID m_UniqueID;
private:

};