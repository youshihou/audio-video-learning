#include "ffmpegs.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}


#define ERROR_BUFFER(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof(errbuf));


FFmpegs::FFmpegs() {

}


static int checkSampleFmt(const AVCodec* codec, enum AVSampleFormat sampleFmt) {
    const enum AVSampleFormat *p = codec->sample_fmts;
    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sampleFmt) {
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


void FFmpegs::accEncode(AudioEncodeSpec &in, const char *outFilename) {
    QFile inFile(in.filename);
    QFile outFile(outFilename);
    int ret = 0;
    AVCodec *codec = nullptr;
    AVCodecContext *ctx = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *pkt = nullptr;

    codec = avcodec_find_encoder_by_name("libfdk_aac");
    if (!codec) {
        qDebug() << "encoder not found";
        return;
    }
    if (!checkSampleFmt(codec, in.sampleFmt)) {
        qDebug() << "unsupported sample format" << av_get_sample_fmt_name(in.sampleFmt);
        return;
    }
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context3 error";
        return;
    }

    ctx->sample_rate = in.sampleRate;
    ctx->sample_fmt = in.sampleFmt;
    ctx->channel_layout = in.chLayout;
    ctx->bit_rate = 32000;
    ctx->profile = FF_PROFILE_AAC_HE_V2;

//    AVDictionary *options = nullptr;
//    av_dict_set(&options, "vbr", "1", 0);
//    ret = avcodec_open2(ctx, codec, &options);
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
    frame->nb_samples = ctx->frame_size;
    frame->format = ctx->sample_fmt;
    frame->channel_layout = ctx->channel_layout;
    ret = av_frame_get_buffer(frame, 0);
    if (ret) {
        ERROR_BUFFER(ret);
        qDebug() << "av_frame_get_buffer error" << errbuf;
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

    while ((ret = inFile.read((char*)frame->data[0], frame->linesize[0])) > 0) {
        if (encode(ctx, frame, pkt, outFile) < 0) {
            goto end;
        }
    }

    encode(ctx, nullptr, pkt, outFile);

end:
    inFile.close();
    outFile.close();

    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);

}

