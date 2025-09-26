#include "GLSounds.h"
#include "GLUtils.h"
#include "GLDataToWave.h"
#include <stdint.h>
#include <cstring>
#include <memory>
#include <vector>
#include "GLBufferReader.h"
#include "GLSoundsQtImp.h"
#include <QDebug>
#include <QAudioDeviceInfo>


void updateFormat(GL::WaveData &output) {
    output.format.setCodec("audio/pcm");
    output.format.setChannelCount(output.numChannels);
    output.format.setSampleSize(output.sampleSize);
    output.format.setSampleRate(output.sampleRate);
    output.format.setSampleType(QAudioFormat::UnSignedInt);

    if (!output.format.isValid()) {
        qWarning() << "Audio format is invalid.";
        return false;
    }

    const QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(output.format)) {
        qWarning() << "Audio format is not supported.";
        return false;
    }
}

struct Context {
    GL::WaveData wavs[kMaxSounds];
};

void GL::Sounds::initContext()
{
    context = new Context;
}

void GL::Sounds::play(int which, int where) {
    Context *ctx = static_cast<Context*>(context);
    bool didPlay = false;
    const GL::WaveData& data = ctx->wavs[which];
    for (auto& out : ctx->outs) {
        if (out->play(data)) {
            didPlay = true;
            break;
        }
    }
    if (!didPlay) {
        GL::SoundsQtImp* imp = new GL::SoundsQtImp;
        imp->play(data);
        ctx->outs.push_back(imp);
    }
}

void GL::Sounds::load(int which, const unsigned char *buf, unsigned bufLen) {
    Context *ctx = static_cast<Context*>(context);
    if (!dataToWave(buf, bufLen, ctx->wavs[which])) {
        qWarning("Can't load sound %d", which);
    }
    updateFormat(ctx->wavs[which]);
}
