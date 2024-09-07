#include "AudioModule.h"

#include <Windows.h>

AudioModule* AudioModule::s_Instance = nullptr;

AudioModule::AudioModule()
{
    s_Instance = this;
}

AudioModule::~AudioModule()
{

}

void AudioModule::PlayAsyncSound(std::string filePath)
{
    PlaySound((LPCTSTR)SND_ALIAS_SYSTEMHAND, NULL, SND_ALIAS_ID | SND_ASYNC);
}
