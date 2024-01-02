#pragma once

#include <string>

class AudioModule
{
public:
    AudioModule();
    ~AudioModule();

    void PlayAsyncSound(std::string filePath);

    static AudioModule* Get() { return s_Instance; }

private:


    static AudioModule* s_Instance;

};

