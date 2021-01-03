#pragma once

class GUIDGen
{

public:
    static unsigned int Generate();
private:
    static unsigned int currentGUID;
};