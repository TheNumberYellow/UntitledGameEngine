#pragma once

#define GUID unsigned int

class GUIDGen
{

public:
    static GUID Generate();
private:
    static GUID currentGUID;
};