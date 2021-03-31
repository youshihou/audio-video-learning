#include "audiothread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#ifdef Q_OS_MAC
    #define FORMAT_NAME "avfoundation"
    #define DEVICE_NAME ":0"
    #define FILE_PATH "/Users/ankui/Desktop/"
#else

#endif


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

void showSpec(AVFormatContext *ctx) {
    AVStream *stream = ctx->streams[0];
    AVCodecParameters *params = stream->codecpar;
    qDebug() << params->channels;
    qDebug() << params->sample_rate;
    qDebug() << params->format;
    qDebug() << av_get_bytes_per_sample((AVSampleFormat) params->format);
}

void AudioThread::run() {
    qDebug() << this << "start......";

    AVInputFormat *fmt = av_find_input_format(FORMAT_NAME);
    if (!fmt) {
        qDebug() << "av_find_input_format error" << FORMAT_NAME;
        return;
    }

    AVFormatContext *ctx = nullptr;
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "avformat_open_input" << errbuf;
        return;
    }

    showSpec(ctx);

    QString filename = FILE_PATH;
    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    filename += ".pcm";

    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << filename;
        avformat_close_input(&ctx);
        return;
    }

    AVPacket pkt;
    while (!isInterruptionRequested()) {
        ret = av_read_frame(ctx, &pkt);
        if (ret == 0) {
            file.write((const char *) pkt.data, pkt.size);
        } else if (ret == AVERROR(EAGAIN)) {
            continue;
        } else {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "av_read_frame error" << errbuf;
            break;
        }
    }

    file.close();
    avformat_close_input(&ctx);
    qDebug() << this << "end......";
}

void AudioThread::setStop(bool stop) {
    _stop = stop;
}
