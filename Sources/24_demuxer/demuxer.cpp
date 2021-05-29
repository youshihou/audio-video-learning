#include "demuxer.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}


#define ERROR_BUFFER \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof(errbuf));

#define END(func) \
    if (ret < 0) { \
        ERROR_BUFFER; \
        qDebug() << #func << "error" << errbuf; \
        goto end; \
    }

#define RET(func) \
    if (ret < 0) { \
        ERROR_BUFFER; \
        qDebug() << #func << "error" << errbuf; \
        return ret; \
    }



Demuxer::Demuxer()
{

}


void Demuxer::demux(const char *inFilename, AudioDecodeSpec &aOut, VideoDecodeSpec &vOut) {
    _aOut = &aOut;
    _vOut = &vOut;

    int ret = 0;
    AVPacket pkt;

    ret = avformat_open_input(&_fmtCtx, inFilename, nullptr, nullptr);
    END(avformat_open_input);

    ret = avformat_find_stream_info(_fmtCtx, nullptr);
    END(avformat_find_stream_info);

    av_dump_format(_fmtCtx, 0, inFilename, 0);
//    fflush(stderr);

    ret = initAudioInfo();
    if (ret < 0) {
        goto end;
    }

    ret = initVideoInfo();
    if (ret < 0) {
        goto end;
    }

    _frame = av_frame_alloc();
    if (!_frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;
    while (av_read_frame(_fmtCtx, &pkt) == 0) {
        if (pkt.stream_index == _aStreamIdx) {
            ret = decode(_aDecodeCtx, &pkt);
        } else if (pkt.stream_index == _vStreamIdx) {
            ret = decode(_vDecodeCtx, &pkt);
        }
        av_packet_unref(&pkt);
        if (ret < 0) {
            goto end;
        }
    }

    decode(_aDecodeCtx, nullptr);
    decode(_vDecodeCtx, nullptr);

end:
    _aOutFile.close();
    _vOutFile.close();
    avcodec_free_context(&_aDecodeCtx);
    avcodec_free_context(&_vDecodeCtx);
    avformat_close_input(&_fmtCtx);
    av_frame_free(&_frame);
}

int Demuxer::initAudioInfo() {
    int ret = initDecoder(&_aStreamIdx, &_aDecodeCtx, AVMEDIA_TYPE_AUDIO);
    if (ret < 0) {
        return ret;
    }

    _aOutFile.setFileName(_aOut->filename);
    if (!_aOutFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << _aOut->filename;
        return -1;
    }
    _aOut->sampleRate = _aDecodeCtx->sample_rate;
    _aOut->sampleFmt = _aDecodeCtx->sample_fmt;
    _aOut->chLayout = _aDecodeCtx->channel_layout;

    return 0;
}

int Demuxer::initVideoInfo() {
    int ret = initDecoder(&_vStreamIdx, &_vDecodeCtx, AVMEDIA_TYPE_VIDEO);
    if (ret < 0) {
        return ret;
    }

    _vOutFile.setFileName(_vOut->filename);
    if (!_vOutFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << _vOut->filename;
        return -1;
    }
    _vOut->width = _vDecodeCtx->width;
    _vOut->height = _vDecodeCtx->height;
    _vOut->pixFmt = _vDecodeCtx->pix_fmt;
    _vOut->fps = _vDecodeCtx->framerate.num;

    return 0;
}

int Demuxer::initDecoder(int* streamIdx, AVCodecContext** decodeCtx, AVMediaType type) {
    int ret = av_find_best_stream(_fmtCtx, type, -1, -1, nullptr, 0);
    RET(av_find_best_stream);

    *streamIdx = ret;
    AVStream *stream = _fmtCtx->streams[*streamIdx];
    if (!stream) {
        qDebug() << "stream is empty";
        return -1;
    }

    AVCodec *decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        qDebug() << "decode not found" << stream->codecpar->codec_id;
        return -1;
    }

    *decodeCtx = avcodec_alloc_context3(decoder);
    if (!*decodeCtx) {
        qDebug() << "avcodec_alloc_context3 error";
        return -1;
    }

    ret = avcodec_parameters_to_context(*decodeCtx, stream->codecpar);
    RET(avcodec_parameters_to_context);

    ret = avcodec_open2(*decodeCtx, decoder, nullptr);
    RET(avcodec_open2);

    return 0;
}

int Demuxer::decode(AVCodecContext *decodeCtx, AVPacket *pkt) {
    int ret = avcodec_send_packet(decodeCtx, pkt);
    RET(avcodec_send_packet);

    while (true) {
        ret = avcodec_receive_frame(decodeCtx, _frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }
        RET(avcodec_receive_frame);

//        if (pkt->stream_index == _aStreamIdx) {
        if (decodeCtx->codec->type == AVMEDIA_TYPE_AUDIO) {
            writeAudioFrame();
        } else if (decodeCtx->codec->type == AVMEDIA_TYPE_VIDEO) {
            writeVideoFrame();
        }
    }
}

void Demuxer::writeAudioFrame() {

}

void Demuxer::writeVideoFrame() {
//    _vOutFile.write((char*)_frame->data[0], _frame->linesize[0] * _vOut->height);
//    _vOutFile.write((char*)_frame->data[1], _frame->linesize[1] * _vOut->height >> 1);
//    _vOutFile.write((char*)_frame->data[2], _frame->linesize[2] * _vOut->height >> 1);

    int imageSize = av_image_get_buffer_size(_vOut->pixFmt, _vOut->width, _vOut->height, 1);
    _vOutFile.write((char*)_frame->data[0], imageSize);
}

