#pragma once

#include <string>
#include <filesystem>

class FilePath
{
public:

    FilePath()
    {
        SetPath("");
    }

    FilePath(const char* InPath)
        : FilePath(std::string(InPath))
    {

    }

    FilePath(std::string InPath)
    {
        SetPath(InPath);
    }

    void SetPath(std::string InPath)
    {
        // Always use absolute path
        std::filesystem::path relPath = InPath;
        std::filesystem::path absPath = std::filesystem::absolute(relPath);

        Path = absPath.generic_string();
    }

    std::string GetFullPath()
    {
        return Path;
    }

    std::string GetFileName()
    {
        size_t LastSlash = Path.find_last_of('/');
        return Path.substr(LastSlash + 1);
    }

    std::string GetFileNameNoExt()
    {
        size_t LastSlash = Path.find_last_of('/');
        size_t LastPeriod = Path.find_last_of('.');

        return Path.substr(LastSlash + 1, LastPeriod - (LastSlash + 1));
    }

    std::string GetExt()
    {
        size_t LastPeriod = Path.find_last_of('.');
        return Path.substr(LastPeriod);
    }

private:

    std::string Path;
};