#include "ffmpegs.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}


#define ERROR_BUFFER(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof(errbuf));


FFmpegs::FFmpegs() {

}


static int checkPixFmt(const AVCodec* codec, enum AVPixelFormat pixFmt) {
    const enum AVPixelFormat *p = codec->pix_fmts;
    while (*p != AV_PIX_FMT_NONE) {
        if (*p == pixFmt) {
            return 1;
        }
        p++;
    }
    return 0;
}


static int encode(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt, QFile &outFile) {
    int ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        ERROR_BUFFER(ret);
        qDebug() << "avcodec_send_frame error" << errbuf;
        return ret;
    }

    while (true) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            return ret;
        }
        outFile.write((char *) pkt->data, pkt->size);

        av_packet_unref(pkt);
    }
}


void FFmpegs::h264Encode(VideoEncodeSpec &in, const char *outFilename) {
    QFile inFile(in.filename);
    QFile outFile(outFilename);
    int ret = 0;
    int imageSize = av_image_get_buffer_size(in.pixFmt, in.width, in.height, 1);

    AVCodec *codec = nullptr;
    AVCodecContext *ctx = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *pkt = nullptr;

    codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
        qDebug() << "encoder not found";
        return;
    }
    if (!checkPixFmt(codec, in.pixFmt)) {
        qDebug() << "unsupported sample format" << av_get_pix_fmt_name(in.pixFmt);
        return;
    }
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context3 error";
        return;
    }

    ctx->width = in.width;
    ctx->height = in.height;
    ctx->pix_fmt = in.pixFmt;
    ctx->time_base = {1, in.fps};

    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        ERROR_BUFFER(ret);
        qDebug() << "avcodec_open2 error" << errbuf;
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    frame->width = ctx->width;
    frame->height = ctx->height;
    frame->format = ctx->pix_fmt;
    frame->pts = 0;

//    ret = av_frame_get_buffer(frame, 0);
//    if (ret < 0) {
//        ERROR_BUFFER(ret);
//        qDebug() << "av_frame_get_buffer error" << errbuf;
//        return;
//        goto end;
//    }

    ret = av_image_alloc(frame->data, frame->linesize, in.width, in.height, in.pixFmt, 1);
    if (ret < 0) {
        ERROR_BUFFER(ret);
        qDebug() << "av_image_alloc error" << errbuf;
        return;
        goto end;
    }


    pkt = av_packet_alloc();
    if (!pkt) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << in.filename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << outFilename;
        goto end;
    }

    while ((ret = inFile.read((char*)frame->data[0], imageSize)) > 0) {
        if (encode(ctx, frame, pkt, outFile) < 0) {
            goto end;
        }
        frame->pts++;
    }

    encode(ctx, nullptr, pkt, outFile);

end:
    inFile.close();
    outFile.close();

    if (frame) {
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
    }
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);

}

