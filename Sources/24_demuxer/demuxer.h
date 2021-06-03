#ifndef DEMUXER_H
#define DEMUXER_H

#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}


typedef struct {
    const char* filename;
    int sampleRate;
    AVSampleFormat sampleFmt;
    int chLayout;
} AudioDecodeSpec;

typedef struct {
    const char* filename;
    int width;
    int height;
    AVPixelFormat pixFmt;
    int fps;
} VideoDecodeSpec;


class Demuxer
{
public:
    Demuxer();

    void demux(const char* inFilename, AudioDecodeSpec &aOut, VideoDecodeSpec &vOut);

private:
    AVFormatContext *_fmtCtx = nullptr;
    AVCodecContext *_aDecodeCtx = nullptr, *_vDecodeCtx = nullptr;
    int _aStreamIdx = 0, _vStreamIdx = 0;
    AudioDecodeSpec *_aOut = nullptr;
    VideoDecodeSpec *_vOut = nullptr;
    QFile _aOutFile, _vOutFile;
    AVFrame *_frame = nullptr;
    uint8_t *_imgbuf[4] = { nullptr };
    int _imglinesize[4] = { 0 };
    int _imgsize = 0;


    int initAudioInfo();
    int initVideoInfo();
    int initDecoder(int* streamIdx, AVCodecContext** decodeCtx, AVMediaType type);
    int decode(AVCodecContext *decodeCtx, AVPacket *pkt, void(Demuxer::*func)());
    void writeAudioFrame();
    void writeVideoFrame();
};

#endif // DEMUXER_H
