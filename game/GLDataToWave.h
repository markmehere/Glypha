#ifndef GL_DATATOWAVE_H
#define GL_DATATOWAVE_H

#include <stdint.h>
#ifdef EMSCRIPTEN
#include "audio.h"
#else
#include <QAudioFormat>
#endif

namespace GL {

    struct WaveData {
#ifndef EMSCRIPTEN
        QAudioFormat format;
#else
        Audio *audio;
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
