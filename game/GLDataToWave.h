#ifndef GL_DATATOWAVE_H
#define GL_DATATOWAVE_H

#include <stdint.h>

#if defined(EMSCRIPTEN)
#include "audio.h"
#elif defined(__ANDROID__)
typedef void *Audio;
#else
#include <QAudioFormat>
#endif

namespace GL {

    struct WaveData {
#if defined(EMSCRIPTEN) || defined(__ANDROID__)
        Audio *audio;
#else
        QAudioFormat format;
#endif
        uint16_t numChannels;
        uint32_t numSampleFrames;
        uint16_t sampleSize;
        uint32_t sampleRate;
        char *data;
        size_t dataLen;
        bool playing;
        WaveData()
            : data(0)
            , dataLen(0)
            , playing(false)
        {
        }

        WaveData(const WaveData& other) = delete;
    };

}

extern bool dataToWave(const uint8_t *data, unsigned dataLen, GL::WaveData &output);

#endif
