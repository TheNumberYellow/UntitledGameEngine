#pragma once

#include <string>
#include <vector>

namespace StringUtils
{
    std::vector<std::string> Split(std::string inputString, std::string delimiter);
    bool Contains(std::string inputString, std::string subString);
}