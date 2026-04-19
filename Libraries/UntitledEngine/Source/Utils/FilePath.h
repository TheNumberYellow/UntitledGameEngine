#pragma once

#include <string>
#include <filesystem>
#include <cassert>

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
        std::filesystem::path path = InPath;
        if (path.is_absolute())
        {
            AbsolutePath = path.generic_string();
            std::filesystem::path currentPath = std::filesystem::current_path();
            RelativePath = std::filesystem::relative(
                AbsolutePath,
                currentPath
            ).generic_string();
        }
        else if (path.is_relative())
        {
            RelativePath = path.generic_string();
            AbsolutePath = std::filesystem::absolute(path).generic_string();
        }
        else
        {
            assert(false && "Path is not absolute or relative");
        }
    }

    std::string GetFullPath()
    {
        return AbsolutePath;
    }

    std::string GetRelativePath()
    {
        return RelativePath;
    }

    std::string GetFileName()
    {
        size_t LastSlash = AbsolutePath.find_last_of('/');
        return AbsolutePath.substr(LastSlash + 1);
    }

    std::string GetFileNameNoExt()
    {
        size_t LastSlash = AbsolutePath.find_last_of('/');
        size_t LastPeriod = AbsolutePath.find_last_of('.');

        return AbsolutePath.substr(LastSlash + 1, LastPeriod - (LastSlash + 1));
    }

    std::string GetExt()
    {
        size_t LastPeriod = AbsolutePath.find_last_of('.');
        return AbsolutePath.substr(LastPeriod);
    }

private:

    std::string AbsolutePath;
    std::string RelativePath;
};