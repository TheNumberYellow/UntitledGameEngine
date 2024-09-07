#pragma once

#include <string>
#include <vector>

struct Node
{


};

struct TaggedData
{
    std::string Tag;
    std::string Data;
};

class TaggedFile
{
public:

    TaggedFile();
    TaggedFile(std::string FileName);

    void Load(std::string FileName);
    void Save(std::string FileName);

private:

    std::vector<TaggedData> Data;
};