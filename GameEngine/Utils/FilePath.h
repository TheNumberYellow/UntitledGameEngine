#pragma once

#include <string>

class FilePath
{
public:

    FilePath()
    {
        SetPath("");
    }

    FilePath(std::string InPath)
    {
        SetPath(InPath);
    }

    void SetPath(std::string InPath)
    {
        Path = InPath;
    }

    std::string GetFullPath()
    {
        return Path;
    }

    std::string GetExt()
    {
        size_t LastPeriod = Path.find_last_of('.');
        return Path.substr(LastPeriod);
    }

private:

    std::string Path;
};