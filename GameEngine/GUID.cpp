#include "GUID.hpp"

unsigned int GUIDGen::currentGUID = 0;

unsigned int GUIDGen::Generate()
{
    return currentGUID++;
}
