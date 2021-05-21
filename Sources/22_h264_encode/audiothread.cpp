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
    VideoEncodeSpec in;
    in.filename = "/Users/ankui/Desktop/zzz/yuv/out.yuv";
    in.width = 848;
    in.height = 464;
    in.fps = 30;
    in.pixFmt = AV_PIX_FMT_YUV420P;

    FFmpegs::h264Encode(in, "/Users/ankui/Desktop/zzz/yuv/out.h264");
}

