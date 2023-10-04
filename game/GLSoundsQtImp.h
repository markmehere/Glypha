#ifndef GLSOUNDSQTIMP_H
#define GLSOUNDSQTIMP_H

#include <QObject>
#include <QByteArray>
#include <QBuffer>
#include <QAudioFormat>
#include <QAudioOutput>

namespace GL {

class SoundsQtImp : public QObject {
    Q_OBJECT
public:
    SoundsQtImp();

    bool play(const WaveData& wave);

private:
    SoundsQtImp(const WaveData& other) = delete;

    bool open_;
    QByteArray bytes_;
    QBuffer* buffer_;
    QAudioOutput* audioOutput_;
    bool playing_;

private slots:
    void handleStateChanged(QAudio::State state);
};

} // namespace

#endif // GLSOUNDSQTIMP_H
