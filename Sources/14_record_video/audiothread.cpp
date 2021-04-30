#include "audiothread.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

#ifdef Q_OS_MAC
    #define FORMAT_NAME "avfoundation"
    #define DEVICE_NAME "0"
    #define FILE_PATH "/Users/ankui/Desktop/zzz/yuv/out.yuv"
#else

#endif


#define ERROR_BUFFER(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof(errbuf));


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
    qDebug() << this << "start......";

    AVInputFormat *fmt = av_find_input_format(FORMAT_NAME);
    if (!fmt) {
        qDebug() << "av_find_input_format error" << FORMAT_NAME;
        return;
    }

    AVFormatContext *ctx = nullptr;

    AVDictionary *options = nullptr;
    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "pixel_format", "yuyv422", 0);
    av_dict_set(&options, "framerate", "30", 0);

    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, &options);
    if (ret < 0) {
        ERROR_BUFFER(ret);
        qDebug() << "avformat_open_input" << errbuf;
        return;
    }


    QFile file(FILE_PATH);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << FILE_PATH;
        avformat_close_input(&ctx);
        return;
    }


    AVCodecParameters *params = ctx->streams[0]->codecpar;
    AVPixelFormat pix_fmt = (AVPixelFormat)params->format;
    /*int pix_size = av_get_bits_per_pixel(av_pix_fmt_desc_get(pix_fmt)) >> 3;
    int image_size = params->width * params->height * pix_size*/;

    int image_size = av_image_get_buffer_size(pix_fmt, params->width, params->height, 1);

    AVPacket *pkt = av_packet_alloc();
    while (!isInterruptionRequested()) {
        ret = av_read_frame(ctx, pkt);
        if (ret == 0) {
//            file.write((const char *) pkt->data, pkt->size);
            file.write((const char *) pkt->data, image_size);
            qDebug() << pkt->size << image_size;
            av_packet_unref(pkt);
        } else if (ret == AVERROR(EAGAIN)) {
            continue;
        } else {
            ERROR_BUFFER(ret);
            qDebug() << "av_read_frame error" << errbuf;
            break;
        }
    }

    av_packet_free(&pkt);

    file.close();

    avformat_close_input(&ctx);

    qDebug() << this << "end......";
}
