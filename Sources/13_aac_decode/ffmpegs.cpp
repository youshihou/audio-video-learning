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

#define IN_DATA_SIZE 20480
#define REFILL_THRESH 4096



FFmpegs::FFmpegs() {

}


static int decode(AVCodecContext* ctx, AVPacket* pkt, AVFrame* frame, QFile &outFile) {
    int ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) {
        ERROR_BUFFER(ret);
        qDebug() << "avcodec_send_packet error" << errbuf;
        return ret;
    }

    while (true) {
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            ERROR_BUFFER(ret);
            qDebug() << "avcodec_receive_frame error" << errbuf;
            return ret;
        }
        outFile.write((char*)frame->data[0], frame->linesize[0]);
    }
}


void FFmpegs::accDecode(const char *inFilename, AudioDecodeSpec &out) {
    int ret = 0;
    char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    char *inData = inDataArray;
    int inLen = 0;
    int inEnd = 0;

    QFile inFile(inFilename);
    QFile outFile(out.filename);
    AVCodec *codec = nullptr;
    AVCodecContext *ctx = nullptr;
    AVCodecParserContext *parserCtx = nullptr;
    AVPacket *pkt = nullptr;
    AVFrame *frame = nullptr;

    codec = avcodec_find_decoder_by_name("libfdk_aac");
    if (!codec) {
        qDebug() << "decoder not found";
        return;
    }

    parserCtx = av_parser_init(codec->id);
    if (!parserCtx) {
        qDebug() << "av_parser_init error";
        return;
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context3 error";
        goto end;
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        qDebug() << "av_packet_alloc error";
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        ERROR_BUFFER(ret);
        qDebug() << "avcodec_open2 error" << errbuf;
        goto end;
    }

    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << inFilename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << out.filename;
        goto end;
    }

    inLen = inFile.read(inData, IN_DATA_SIZE);
    while (inLen > 0) {
        ret = av_parser_parse2(parserCtx, ctx, &pkt->data, &pkt->size, (uint8_t*)inData, inLen, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            ERROR_BUFFER(ret);
            qDebug() << "av_parser_parse2 error" << errbuf;
            goto end;
        }

        inData += ret;
        inLen -= ret;

        if (pkt->size <= 0) { continue; }

        if (decode(ctx, pkt, frame, outFile) < 0) {
            goto end;
        }

        if (inLen < REFILL_THRESH && !inEnd) {
            memmove(inDataArray, inData, inLen);
            inData = inDataArray;
            int len = inFile.read(inData + inLen, IN_DATA_SIZE - inLen);
            if (len > 0) {
                inLen += len;
            } else if (len == 0) {
                inEnd = 1;
            }
        }
    }

    decode(ctx, nullptr, frame, outFile);

    out.sampleRate = ctx->sample_rate;
    out.sampleFmt = ctx->sample_fmt;
    out.chLayout = ctx->channel_layout;

end:
    inFile.close();
    outFile.close();
    av_packet_free(&pkt);
    av_frame_free(&frame);
    av_parser_close(parserCtx);
    avcodec_free_context(&ctx);
}

