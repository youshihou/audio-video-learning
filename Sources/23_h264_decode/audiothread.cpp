#include "audiothread.h"

#include <QDebug>
#include "ffmpegs.h"


extern "C" {
#include <libavutil/imgutils.h>
}

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
    VideoDecodeSpec out;
    out.filename = "/Users/ankui/Desktop/zzz/yuv/out_new.yuv";

    FFmpegs::h264Decode("/Users/ankui/Desktop/zzz/yuv/out.h264", out);

    qDebug() << out.width << out.height << out.fps << av_get_pix_fmt_name(out.pixFmt);
}

