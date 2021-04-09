#include "ffmpegs.h"

#include <QDebug>
#include <QFile>

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
}


#define ERROR_BUFFER(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof(errbuf));


FFmpegs::FFmpegs() {

}


void FFmpegs::resampleAudio(ResampleAudioSpec &in, ResampleAudioSpec &out) {
    resampleAudio(in.filename, in.sampleRate, in.sampleFmt, in.channelLayout,
                  out.filename, out.sampleRate, out.sampleFmt, out.channelLayout);
}

void FFmpegs::resampleAudio(const char *inFilename,
                            int inSampleRate,
                            AVSampleFormat inSampleFmt,
                            int inChLayout,
                            const char *outFilename,
                            int outSampleRate,
                            AVSampleFormat outSampleFmt,
                            int outChLayout) {
    QFile inFile(inFilename);
    QFile outFile(outFilename);

    uint8_t **inData = nullptr;
    int inLineSize = 0;
    int inChannels = av_get_channel_layout_nb_channels(inChLayout);
    int inBytesPerSample = inChannels * av_get_bytes_per_sample(inSampleFmt);
    int inSamples = 1024;

    uint8_t **outData = nullptr;
    int outLineSize = 0;
    int outChannels = av_get_channel_layout_nb_channels(outChLayout);
    int outBytesPerSample = outChannels * av_get_bytes_per_sample(outSampleFmt);
    int outSamples = av_rescale_rnd(outSampleRate, inSamples, inSampleRate, AV_ROUND_UP);

    int ret = 0;
    int len = 0;

    SwrContext *ctx = swr_alloc_set_opts(nullptr,
                                         outChLayout, outSampleFmt, outSampleRate,
                                         inChLayout, inSampleFmt, inSampleRate,
                                         0, nullptr);
    if (!ctx) {
        qDebug() << "swr_alloc_set_opts error";
        goto end;
    }

    ret = swr_init(ctx);
    if (ret < 0) {
        ERROR_BUFFER(ret);
        qDebug() << "swr_init error" << errbuf;
        goto end;
    }


    ret = av_samples_alloc_array_and_samples(&inData, &inLineSize, inChannels, inSamples, inSampleFmt, 1);
    if (ret < 0) {
        ERROR_BUFFER(ret);
        qDebug() << "av_samples_alloc_array_and_samples error" << errbuf;
        goto end;
    }


    ret = av_samples_alloc_array_and_samples(&outData, &outLineSize, outChannels, outSamples, outSampleFmt, 1);
    if (ret < 0) {
        ERROR_BUFFER(ret);
        qDebug() << "av_samples_alloc_array_and_samples error" << errbuf;
        goto end;
    }


    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << inFilename;
        goto end;
    }

    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << outFilename;
        goto end;
    }

    // *(p + i) = p[i]
    // intData[0] = *inData, outData[0] = *outData
    while ((len = inFile.read((char *)inData[0], inLineSize)) > 0) {
        inSamples = len / inBytesPerSample;
        ret = swr_convert(ctx, outData, outSamples, (const uint8_t **)inData, inSamples);
        if (ret < 0) {
            ERROR_BUFFER(ret);
            qDebug() << "swr_convert error" << errbuf;
            goto end;
        }

//        int size = av_samples_get_buffer_size(nullptr, outChannels, ret, outSampleFmt, 1);
//        outFile.write((char *)outData[0], size);

        outFile.write((char *)outData[0], ret * outBytesPerSample);
    }

    while ((ret = swr_convert(ctx, outData, outSamples, nullptr, 0)) > 0) {
        qDebug() << ret;
        outFile.write((char *)outData[0], ret * outBytesPerSample);
    }

 end:
    inFile.close();
    outFile.close();

    if (inData) {
        av_freep(&inData[0]);
    }
    av_freep(&inData);
    if (outData) {
        av_freep(&outData[0]);
    }
    av_freep(&outData);

    swr_free(&ctx);
}
