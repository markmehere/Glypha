#include "GLSounds.h"
#include "GLDataToWave.h"
struct Context {
    GL::WaveData wavs[kMaxSounds];
    bool hasLoaded;
};

void GL::Sounds::initContext()
{
  context = new Context;
}

void GL::Sounds::play(int which, int where)
{
  Context *ctx = static_cast<Context*>(context);
  if (!ctx->hasLoaded) {
    initAudio();
    ctx->hasLoaded = true;
  }
  const GL::WaveData *data = &ctx->wavs[which];
  playSoundFromMemory(data->audio, SDL_MIX_MAXVOLUME, which == kMusicSound, which);
}

void GL::Sounds::load(int which, const unsigned char *buf, unsigned bufLen)
{
  Context *ctx = static_cast<Context*>(context);
  if (!dataToWave(buf, bufLen, ctx->wavs[which])) {
    SDL_Log("Can't load sound %d", which);
  }
  GL::WaveData *wave = &ctx->wavs[which];
  ctx->wavs[which].audio = (Audio *)calloc(1, sizeof(Audio));
  Audio *simple = ctx->wavs[which].audio;
  SDL_AudioSpec *audio = &(ctx->wavs[which].audio->audio);
  audio->freq = wave->sampleRate;
  audio->format = wave->sampleSize == 8 ? AUDIO_U8 : AUDIO_S16;
  audio->channels = wave->numChannels;
  audio->samples = 4096; /* bit dangerous we assume this means we laod the full sound at once */
  simple->lengthTrue = wave->dataLen;
  simple->bufferTrue = (uint8_t *)wave->data;
  simple->length = wave->dataLen;
  simple->buffer = (uint8_t *)wave->data;
}
