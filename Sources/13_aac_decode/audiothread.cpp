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
    AudioDecodeSpec out;
    out.filename = "/Users/ankui/Desktop/zzz/aac/out_new.pcm";

    FFmpegs::accDecode("/Users/ankui/Desktop/zzz/aac/out_new.aac", out);

    qDebug() << out.sampleRate << av_get_sample_fmt_name(out.sampleFmt) << av_get_channel_layout_nb_channels(out.chLayout);
}

