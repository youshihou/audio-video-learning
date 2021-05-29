#include "audiothread.h"

#include <QDebug>
#include "demuxer.h"


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
    AudioDecodeSpec aOut;
    aOut.filename = "/Users/ankui/Desktop/zzz/demux/out.pcm";
    VideoDecodeSpec vout;
    vout.filename = "/Users/ankui/Desktop/zzz/demux/out.yuv";

    Demuxer().demux("/Users/ankui/Desktop/zzz/demux/in.mp4", aOut, vout);

    qDebug() << aOut.sampleRate
             << av_get_channel_layout_nb_channels(aOut.chLayout)
             << av_get_sample_fmt_name(aOut.sampleFmt);
    qDebug() << vout.width << vout.height  << vout.fps
             << av_get_pix_fmt_name(vout.pixFmt);
}

