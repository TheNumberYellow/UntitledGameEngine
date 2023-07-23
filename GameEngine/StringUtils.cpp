#include "StringUtils.h"

std::vector<std::string> StringUtils::Split(std::string inputString, std::string delimiter)
{
    std::vector<std::string> output;

    size_t first = 0;
    size_t last = 0;

    do
    {
        last = inputString.find(delimiter, first);
        std::string p = inputString.substr(first, last - first);
        if (p != "")
        {
            output.push_back(inputString.substr(first, last - first));
        }
        first = last + (delimiter.length());
    } 	while (last != std::string::npos);

    return output;
}
