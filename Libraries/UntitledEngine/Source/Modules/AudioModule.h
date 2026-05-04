#pragma once

#include <string>
#include <cstdint>

#include "al.h"
#include "alc.h"
#include "efx.h"
#include "efx-creative.h"
#include "xram.h"


// WAV file loading stuff - likely want to move this later
struct RIFF_HEADER
{
    char RIFF[4];        // "RIFF"
    uint32_t ChunkSize;    // Size of the entire file minus 8 bytes (RIFF and ChunkSize fields)
    char WAVE[4];        // "WAVE"
};

struct RIFF_CHUNK_HEADER
{
    char ChunkID[4];      // "fmt "
    uint32_t ChunkSize;    // Size of the chunk data
    uint16_t AudioFormat;    // Audio format (1 for PCM)
    uint16_t NumChannels;    // Number of audio channels
    uint32_t SampleRate;    // Sample rate (e.g. 44100)
    uint32_t ByteRate;      // Byte rate (SampleRate * NumChannels * BitsPerSample/8)
    uint16_t BlockAlign;    // Block align (NumChannels * BitsPerSample/8)
    uint16_t BitsPerSample; // Bits per sample (e.g. 16)
};

struct DATA_CHUNK
{
    char ChunkID[4];      // "data"
    uint32_t ChunkSize;    // Size of the audio data
    // Followed by audio data bytes
};

struct AudioBuffer
{
    ALuint BufferID;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint16_t BitsPerSample;
};

struct AudioSource
{
    ALuint SourceID;
    float Volume;
    float Pitch;
    bool Looping;
};

class AudioModule
{
public:
    AudioModule();
    ~AudioModule();

    //void PlayAsyncSound(std::string filePath);

    AudioBuffer LoadWaveFile(std::string filePath);
    AudioSource CreateAudioSource(const AudioBuffer& buffer, float volume = 1.0f, float pitch = 1.0f, bool looping = false);

    void PlayAudioSource(const AudioSource& source);

    static AudioModule* Get() { return s_Instance; }

private:


    static AudioModule* s_Instance;

};