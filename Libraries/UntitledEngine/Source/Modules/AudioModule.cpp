#include "AudioModule.h"

#include "GameEngine.h"

#include <Windows.h>
#include <fstream>


AudioModule* AudioModule::s_Instance = nullptr;

AudioModule::AudioModule()
{
    ALCdevice* device = alcOpenDevice(nullptr);

    if (!device)
    {
        Engine::FatalError("Failed to open audio device!");
        return;
    }

    ALCcontext* context = alcCreateContext(device, nullptr);
    
    if (!context)
    {
        Engine::FatalError("Failed to create audio context!");
        return;
    }

    alcMakeContextCurrent(context);

    char* alBuffer;
    ALenum alFormatBuffer;
    ALsizei alFreqBuffer;
    long alBUfferLen;
    ALboolean alLoop;
    unsigned int alSource;
    unsigned int alSampleSet;

    
    //alutCreateBufferFromFile("Assets/Sounds/test.wav", &alBuffer);


    s_Instance = this;
}

AudioModule::~AudioModule()
{

    ALCcontext* context = alcGetCurrentContext();
    ALCdevice* device = alcGetContextsDevice(context);
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);

}

//void AudioModule::PlayAsyncSound(std::string filePath)
//{
//    PlaySoundA(filePath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
//}

AudioBuffer AudioModule::LoadWaveFile(std::string filePath)
{
    AudioBuffer audioBuffer;

    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open())
    {
        Engine::FatalError("Failed to open wave file: " + filePath);
        return audioBuffer;
    }
    
    // Read RIFF header
    RIFF_HEADER riffHeader;
    file.read(reinterpret_cast<char*>(&riffHeader), sizeof(RIFF_HEADER));

    if (std::string(riffHeader.RIFF, 4) != "RIFF" || std::string(riffHeader.WAVE, 4) != "WAVE")
    {
        Engine::FatalError("Invalid WAV file format: " + filePath);
        return audioBuffer;
    }

    // Read RIFF chunk header
    RIFF_CHUNK_HEADER chunkHeader;
    file.read(reinterpret_cast<char*>(&chunkHeader), sizeof(RIFF_CHUNK_HEADER));

    if (std::string(chunkHeader.ChunkID, 4) != "fmt ")
    {
        Engine::FatalError("Expected 'fmt ' chunk in WAV file: " + filePath);
        return audioBuffer;
    }

    // Read audio data chunk header
    DATA_CHUNK dataChunk;
    file.read(reinterpret_cast<char*>(&dataChunk), sizeof(DATA_CHUNK));

    if (std::string(dataChunk.ChunkID, 4) != "data")
    {
        Engine::FatalError("Expected 'data' chunk in WAV file: " + filePath);
        return audioBuffer;
    }

    // Read audio data
    std::vector<char> audioData(dataChunk.ChunkSize);
    file.read(audioData.data(), dataChunk.ChunkSize);

    alGenBuffers(1, &audioBuffer.BufferID);
    audioBuffer.NumChannels = chunkHeader.NumChannels;
    audioBuffer.SampleRate = chunkHeader.SampleRate;
    audioBuffer.BitsPerSample = chunkHeader.BitsPerSample;

    alBufferData(audioBuffer.BufferID, 
                 (audioBuffer.NumChannels == 1) ? ((audioBuffer.BitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16) :
                 ((audioBuffer.BitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16),
                 audioData.data(), 
                 static_cast<ALsizei>(audioData.size()), 
        audioBuffer.SampleRate);

    // Free the data buffer memory
    audioData.clear();

    file.close();

    return audioBuffer;
}

AudioSource AudioModule::CreateAudioSource(const AudioBuffer& buffer, float volume, float pitch, bool looping)
{
    AudioSource source;
    alGenSources(1, &source.SourceID);
    alSourcei(source.SourceID, AL_BUFFER, buffer.BufferID);
    alSourcef(source.SourceID, AL_GAIN, volume);
    alSourcef(source.SourceID, AL_PITCH, pitch);
    alSourcei(source.SourceID, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
    return source;
}

void AudioModule::PlayAudioSource(const AudioSource& source)
{
    alSourcePlay(source.SourceID);
}
