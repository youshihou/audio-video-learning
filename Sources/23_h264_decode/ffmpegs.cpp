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

#define IN_DATA_SIZE 4096



FFmpegs::FFmpegs() {

}


static int frameIdx = 0;

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

        qDebug() << "the frame: " << ++frameIdx;
//        qDebug() << frame->data[0] << frame->data[1] << frame->data[2];

        /*
            imageSize 590208
            frame->data[0] 0x7fba76d88000  d88000
            frame->data[1] 0x7fba76df2000  df2000
            frame->data[2] 0x7fba76e0d000  e0d000
            frame->data[1] - frame->data[0] = 434176  y
            frame->data[2] - frame->data[1] = 110592  u

            frame->linesize[0] = 896
            frame->linesize[1] = 448
            frame->linesize[2] = 448

            y 848*464*1  = 393472
            u 848*464*0.25 = 98368
            v 848*464*0.25
        */

//        outFile.write((char*)frame->data[0], 393472);
//        outFile.write((char*)frame->data[1], 98368);
//        outFile.write((char*)frame->data[2], 98368);

        qDebug() << frame->linesize[0]
                << frame->linesize[1]
                << frame->linesize[2]
                << ctx->width << ctx->height;


//        outFile.write((char*)frame->data[0], ctx->width * ctx->height);
//        outFile.write((char*)frame->data[1], ctx->width * ctx->height >> 2);
//        outFile.write((char*)frame->data[2], ctx->width * ctx->height >> 2);

        outFile.write((char*)frame->data[0], (frame->linesize[0] - 48) * ctx->height);
        outFile.write((char*)frame->data[1], (frame->linesize[1] - 24) * ctx->height >> 1);
        outFile.write((char*)frame->data[2], (frame->linesize[2] - 24) * ctx->height >> 1);

//        int imageSize = av_image_get_buffer_size(ctx->pix_fmt, ctx->width, ctx->height, 1);
//        qDebug() << imageSize;
//        outFile.write((char*)frame->data[0], imageSize);

//        outFile.write((char*)frame->data[0], frame->linesize[0] * ctx->height);
//        outFile.write((char*)frame->data[1], frame->linesize[1] * ctx->height >> 1);
//        outFile.write((char*)frame->data[2], frame->linesize[2] * ctx->height >> 1);
    }
}


void FFmpegs::h264Decode(const char *inFilename, VideoDecodeSpec &out) {
    int ret = 0;
    char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    char *inData = nullptr;
    int inLen = 0;
    int inEnd = 0;

    QFile inFile(inFilename);
    QFile outFile(out.filename);
    AVCodec *codec = nullptr;
    AVCodecContext *ctx = nullptr;
    AVCodecParserContext *parserCtx = nullptr;
    AVPacket *pkt = nullptr;
    AVFrame *frame = nullptr;

//    codec = avcodec_find_decoder_by_name("h264");
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
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

//    av_image_alloc(frame->data, frame->linesize, 848, 464, AV_PIX_FMT_YUV420P, 1);
//    qDebug() << frame->data[0] << frame->data[1] << frame->data[2];
//    qDebug() << frame->linesize[0] << frame->linesize[1] << frame->linesize[2];
//    /*
//        0x7f9b944a4000  4a4000   393472
//        0x7f9b94504100  504100
//        0x7f9b9451c140  51c140   98368

//        848 424 424
//    */
//    return;

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

    do {
        inLen = inFile.read(inDataArray, IN_DATA_SIZE);
        inEnd = !inLen;
        inData = inDataArray;
        while (inLen > 0 || inEnd) {
            ret = av_parser_parse2(parserCtx, ctx, &pkt->data,
                                   &pkt->size, (uint8_t*)inData,
                                   inLen, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                ERROR_BUFFER(ret);
                qDebug() << "av_parser_parse2 error" << errbuf;
                goto end;
            }

            inData += ret;
            inLen -= ret;

            qDebug() << inEnd << pkt->size << ret;

            if (pkt->size > 0 && decode(ctx, pkt, frame, outFile) < 0) {
                goto end;
            }

            if (inEnd) {
                break;
            }
        }

    } while (!inEnd);


    // 少一帧
//    while ((inLen = inFile.read(inDataArray, IN_DATA_SIZE)) > 0) {
//        inData = inDataArray;
//        while (inLen > 0) {
//            ret = av_parser_parse2(parserCtx, ctx, &pkt->data, &pkt->size, (uint8_t*)inData, inLen, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
//            if (ret < 0) {
//                ERROR_BUFFER(ret);
//                qDebug() << "av_parser_parse2 error" << errbuf;
//                goto end;
//            }

//            inData += ret;
//            inLen -= ret;

//            if (pkt->size > 0 && decode(ctx, pkt, frame, outFile) < 0) {
//                goto end;
//            }
//        }

//    }

    decode(ctx, nullptr, frame, outFile);

    out.width = ctx->width;
    out.height = ctx->height;
    out.pixFmt = ctx->pix_fmt;
    out.fps = ctx->framerate.num;

end:
    inFile.close();
    outFile.close();
    av_packet_free(&pkt);
    av_frame_free(&frame);
    av_parser_close(parserCtx);
    avcodec_free_context(&ctx);
}

