#include "audiothread.h"

#include <QDebug>
#include "ffmpegs.h"


AudioThread::AudioThread(QObject *parent) : QThread(parent) {
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();
    qDebug() << this << "deStruct";
}

void AudioThread::run() {
    AudioEncodeSpec in;
    in.filename = "/Users/ankui/Desktop/zzz/aac/in.pcm";
    in.sampleRate = 44100;
    in.sampleFmt = AV_SAMPLE_FMT_S16;
    in.chLayout = AV_CH_LAYOUT_STEREO;

    FFmpegs::accEncode(in, "/Users/ankui/Desktop/zzz/aac/out_new.aac");
}

