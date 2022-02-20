#include "GUID.hpp"

GUID GUIDGen::currentGUID = 0;

GUID GUIDGen::Generate()
{
    return currentGUID++;
}
