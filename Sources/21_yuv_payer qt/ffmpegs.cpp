#include "ffmpegs.h"
#include <QDebug>
#include <QFile>


extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}



#define ERR_BUF \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof(errbuf));

#define END(func) \
    if (ret < 0) { \
        ERR_BUF(ret); \
        qDebug() << #func << "error" << errbuf; \
        goto end; \
    }


FFmpegs::FFmpegs()
{

}




void FFmpegs::convertRawVideo(RawVideoFrame &in, RawVideoFrame &out) {
    SwsContext *ctx = nullptr;
    uint8_t *inData[4], *outData[4];
    int inStrides[4], outStrides[4];
    int ret = 0;
    int inFrameSize, outFrameSize;

    ctx = sws_getContext(in.width, in.height, in.format,
                         out.width, out.height, out.format,
                         SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!ctx) {
        qDebug() << "sws_getContext error";
        goto end;
    }

    ret = av_image_alloc(inData, inStrides, in.width, in.height, in.format, 1);
    END(av_image_alloc);
    ret = av_image_alloc(outData, outStrides, out.width, out.height, out.format, 1);
    END(av_image_alloc);

    inFrameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);
    outFrameSize = av_image_get_buffer_size(out.format, out.width, out.height, 1);

    memcpy(inData[0], in.pixels, inFrameSize);
    sws_scale(ctx, inData, inStrides, 0, in.height, outData, outStrides);
    out.pixels = (char *)malloc(outFrameSize);
    memcpy(out.pixels, outData[0], outFrameSize);

end:
    av_freep(&inData[0]);
    av_freep(&outData[0]);
    sws_freeContext(ctx);
}


void FFmpegs::convertRawVideo(RawVideoFile &in, RawVideoFile &out) {
    SwsContext *ctx = nullptr;
    uint8_t *inData[4], *outData[4];
    int inStrides[4], outStrides[4];
    int ret = 0;
    QFile inFile(in.filename), outFile(out.filename);
    int inFrameSize, outFrameSize;
    int frameIdx = 0;

    ctx = sws_getContext(in.width, in.height, in.format,
                         out.width, out.height, out.format,
                         SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!ctx) {
        qDebug() << "sws_getContext error";
        goto end;
    }

    ret = av_image_alloc(inData, inStrides, in.width, in.height, in.format, 1);
    END(av_image_alloc);
    ret = av_image_alloc(outData, outStrides, out.width, out.height, out.format, 1);
    END(av_image_alloc);

    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << in.filename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << out.filename;
        goto end;
    }

    inFrameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);
    outFrameSize = av_image_get_buffer_size(out.format, out.width, out.height, 1);

    while (inFile.read((char *)inData[0], inFrameSize) == inFrameSize) {
        sws_scale(ctx, inData, inStrides, 0, in.height, outData, outStrides);
        outFile.write((char *)outData[0], outFrameSize);
        qDebug() << frameIdx++ << " frame";
    }

end:
    inFile.close();
    outFile.close();
    av_freep(&inData[0]);
    av_freep(&outData[0]);
    sws_freeContext(ctx);
}
