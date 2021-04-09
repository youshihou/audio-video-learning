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
    ResampleAudioSpec in;
    in.filename = "/Users/ankui/Desktop/44100_f32le_2.pcm";
    in.sampleFmt = AV_SAMPLE_FMT_FLT;
    in.sampleRate = 44100;
    in.channelLayout = AV_CH_LAYOUT_STEREO;

    ResampleAudioSpec out;
    out.filename = "/Users/ankui/Desktop/48000_s16le_1.pcm";
    out.sampleFmt = AV_SAMPLE_FMT_S16;
    out.sampleRate = 48000;
    out.channelLayout = AV_CH_LAYOUT_MONO;

    FFmpegs::resampleAudio(in, out);
}

