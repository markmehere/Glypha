#include <aaudio/AAudio.h>
#include "GLSounds.h"
#include "GLDataToWave.h"
#include "AndroidOut.h"

extern bool audioIsMuted;

const int kStreamPoolSize = 16;
const int lowPriorityPool = 4;

const bool isLowPrioritySound[kMaxSounds] = {
    false, // kBirdSound
    false, // kBonusSound
    false, // kBoom1Sound
    false, // kBoom2Sound
    false, // kSplashSound
    true,  // kFlapSound
    true,  // kGrateSound
    false, // kLightningSound
    false, // kMusicSound
    true,  // kScreechSound
    false, // kSpawnSound
    true,  // kWalkSound
    true,  // kFlap2Sound
    false  // kScrape2Sound
};

struct StreamSlot {
    AAudioStream *stream;
    int soundIndex;
    int64_t startTime; // For picking oldest
    bool busy;
    const int16_t *audioData;
    size_t audioLen;      // in frames
    size_t audioOffset;   // current playback position
    float balance;
};

struct Context {
    GL::WaveData wavs[kMaxSounds];
    StreamSlot streams[kStreamPoolSize];
    int32_t sampleRate;
    bool hasLoaded;
    bool streamsCreated;
};

void convertWaveToI16(GL::WaveData &wave) {
    if (wave.sampleSize != 8 || wave.numChannels != 1) return;

    int16_t *newData = new int16_t[wave.numSampleFrames];
    uint8_t *oldData = reinterpret_cast<uint8_t *>(wave.data);

    for (size_t i = 0; i < wave.numSampleFrames; i++) {
        newData[i] = ((int)oldData[i] - 128) * 256;
    }

    delete[] oldData;

    wave.data = reinterpret_cast<char *>(newData);
    wave.dataLen = wave.numSampleFrames * sizeof(int16_t);
    wave.sampleSize = 16;
}

void GL::Sounds::initContext()
{
    context = new Context;
    Context *ctx = static_cast<Context *>(context);
    ctx->hasLoaded = false;
    ctx->sampleRate = -1;
    ctx->streamsCreated = false;
    for (int i = 0; i < kStreamPoolSize; i++) {
        ctx->streams[i].stream = nullptr;
        ctx->streams[i].busy = false;
        ctx->streams[i].soundIndex = -1;
        ctx->streams[i].startTime = 0;
    }
}

// Data callback for non-blocking playback
static aaudio_data_callback_result_t audioDataCallback(
        AAudioStream *stream,
        void *userData,
        void *audioData,
        int32_t numFrames)
{
    StreamSlot *slot = (StreamSlot *)userData;
    if (!slot->busy || !slot->audioData) return AAUDIO_CALLBACK_RESULT_STOP;

    int16_t *out = (int16_t *)audioData;
    size_t framesLeft = slot->audioLen - slot->audioOffset;
    size_t framesToCopy = framesLeft < (size_t)numFrames ? framesLeft : (size_t)numFrames;

#ifdef D3SOUND
    for (int i = 0; i < framesToCopy; i++) {
        auto sample = ((int16_t *)(slot->audioData + slot->audioOffset + i))[0];
        out[i * 2] = ((float)sample * (1.0f - slot->balance / 4.0f));
        out[i * 2 + 1] = ((float)sample * (0.75f + slot->balance / 4.0f));
    }
#else
    memcpy(out, slot->audioData + slot->audioOffset, framesToCopy * sizeof(int16_t));
#endif
    slot->audioOffset += framesToCopy;

    // Zero out remainder if sound finished
    if (framesToCopy < (size_t)numFrames) {
#ifdef D3SOUND
        memset(out + framesToCopy * 2, 0, (numFrames - framesToCopy) * 2 * sizeof(int16_t));
#else
        memset(out + framesToCopy, 0, (numFrames - framesToCopy) * sizeof(int16_t));
#endif
    }

    if (slot->audioOffset >= slot->audioLen) {
        slot->busy = false;
        return AAUDIO_CALLBACK_RESULT_STOP;
    }
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void GL::Sounds::load(int which, const unsigned char *buf, unsigned bufLen)
{
    Context *ctx = static_cast<Context *>(context);
    if (!dataToWave(buf, bufLen, ctx->wavs[which])) {
        aout << "Can't load sound " << which << std::endl;
        return;
    }
    if (ctx->sampleRate == -1) {
        ctx->sampleRate = (int32_t)ctx->wavs[which].sampleRate;
    }
    else if (ctx->sampleRate != (int32_t)ctx->wavs[which].sampleRate) {
        aout << "[FATAL] Unexpected sample rate for sound " << which << std::endl;
        std::terminate();
    }
    else if (!ctx->streamsCreated) {
        for (int i = 0; i < kStreamPoolSize; i++) {
            AAudioStreamBuilder *builder;
            AAudio_createStreamBuilder(&builder);
            AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
#ifdef D3SOUND
            AAudioStreamBuilder_setChannelCount(builder, 2);
#else
            AAudioStreamBuilder_setChannelCount(builder, 1);
#endif
            AAudioStreamBuilder_setSampleRate(builder, ctx->sampleRate);
            AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
            AAudioStreamBuilder_setDataCallback(builder, audioDataCallback, &ctx->streams[i]);

            aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &ctx->streams[i].stream);
            AAudioStreamBuilder_delete(builder);

            if (result != AAUDIO_OK) {
                aout << "AAudioStreamBuilder_openStream failed: " << result << std::endl;
                for (int j = 0; j <= i; j++) {
                    if (ctx->streams[j].stream) {
                        AAudioStream_close(ctx->streams[j].stream);
                        ctx->streams[j].stream = nullptr;
                    }
                }
                ctx->streamsCreated = false;
                return;
            }
        }
        ctx->streamsCreated = true;
    }
    convertWaveToI16(ctx->wavs[which]);
    ctx->hasLoaded = true;
}

static int64_t getTimeMs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

void GL::Sounds::play(int which, int where)
{
    Context *ctx = static_cast<Context *>(context);
    if (!ctx->hasLoaded || !ctx->streamsCreated) {
        aout << "Audio not initialized" << std::endl;
        return;
    }
    GL::WaveData *wave = &ctx->wavs[which];
    if (!wave->data || wave->dataLen == 0) {
        aout << "No sound loaded for " << which << std::endl;
        return;
    }

    // In one loop: find a free stream or the oldest busy one and close streams that are no longer busy
    int slot = -1;
    int backupSlot = 0;
    int64_t oldest = INT64_MAX;
    for (int i = (isLowPrioritySound[which] ? 0 : lowPriorityPool);
             i < (isLowPrioritySound[which] ? lowPriorityPool : kStreamPoolSize);
             i++
    ) {
        if (slot == -1 && !ctx->streams[i].busy) {
            slot = i;
        }
        if (audioIsMuted) {
            aout << "User has muted audio" << std::endl;
            if (AAudioStream_requestStop(ctx->streams[slot].stream) == AAUDIO_OK) {
                ctx->streams[slot].busy = false;
            }
        }
        if (ctx->streams[i].startTime < oldest) {
            oldest = ctx->streams[i].startTime;
            backupSlot = i;
        }
    }
    if (slot == -1) {
        slot = backupSlot;
    }
    if (audioIsMuted) {
        return;
    }

    if (ctx->streams[slot].busy) {
        if (AAudioStream_requestStop(ctx->streams[slot].stream) == AAUDIO_OK) {
            ctx->streams[slot].busy = false;
        }
        aout << "No available for slots for audio; sound omitted for audio safety" << std::endl;
        return;
    }

    const float duration = (float)wave->dataLen / ((float)sizeof(int16_t) * (float)wave->sampleRate);
    aout << "Playing " << which << " on slot " << slot <<
         " ("  << duration << "s @ " <<  wave->sampleRate << "Hz)" << std::endl;

    // Set up playback state
    ctx->streams[slot].busy = true;
    ctx->streams[slot].audioData = (int16_t *)wave->data;
    ctx->streams[slot].audioLen = wave->dataLen / sizeof(int16_t);
    ctx->streams[slot].audioOffset = 0;
    ctx->streams[slot].soundIndex = which;
    ctx->streams[slot].startTime = getTimeMs();
    ctx->streams[slot].balance = std::clamp((float)where / 640.0f, 0.0f, 1.0f);
    aout << "WHERE " << where << std::endl;
    // Start the stream
    aaudio_result_t result = AAudioStream_requestStart(ctx->streams[slot].stream);
    if (result != AAUDIO_OK) {
        aout << "AAudioStream_requestStart failed: " << result << std::endl;
        return;
    }
}

/*
GL::Sounds::~Sounds()
{
    if (context) {
        Context *ctx = static_cast<Context *>(context);
        for (int i = 0; i < kStreamPoolSize; ++i) {
            if (ctx->streams[i].stream) {
                AAudioStream_requestStop(ctx->streams[i].stream);
                AAudioStream_close(ctx->streams[i].stream);
                ctx->streams[i].stream = nullptr;
            }
        }
        // If GL::WaveData needs manual cleanup, do it here
        delete ctx;
        context = nullptr;
    }
}
*/