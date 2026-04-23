#include "AudioModule.h"

#include "GameEngine.h"

#include <Windows.h>

#include"al.h"
#include"alc.h"
#include"efx.h"
#include"efx-creative.h"
#include"xram.h"

AudioModule* AudioModule::s_Instance = nullptr;

AudioModule::AudioModule()
{
    //ALCdevice* device = alcOpenDevice(nullptr);

    //if (!device)
    //{
    //    Engine::FatalError("Failed to open audio device!");
    //    return;
    //}

    //ALCcontext* context = alcCreateContext(device, nullptr);
    //
    //if (!context)
    //{
    //    Engine::FatalError("Failed to create audio context!");
    //    return;
    //}

    //alcMakeContextCurrent(context);

    s_Instance = this;
}

AudioModule::~AudioModule()
{

}

void AudioModule::PlayAsyncSound(std::string filePath)
{
    PlaySoundA(filePath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
}
