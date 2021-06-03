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
    AVPacket *pkt = nullptr;

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


    pkt = av_packet_alloc();
    if (!pkt) {
        qDebug() << "av_packet_alloc error";
        goto end;
    }
    pkt->data = nullptr;
    pkt->size = 0;

    while (av_read_frame(_fmtCtx, pkt) == 0) {
        if (pkt->stream_index == _aStreamIdx) {
            ret = decode(_aDecodeCtx, pkt, &Demuxer::writeAudioFrame);
        } else if (pkt->stream_index == _vStreamIdx) {
            ret = decode(_vDecodeCtx, pkt, &Demuxer::writeVideoFrame);
        }
        av_packet_unref(pkt);
        if (ret < 0) {
            goto end;
        }
    }

    decode(_aDecodeCtx, nullptr, &Demuxer::writeAudioFrame);
    decode(_vDecodeCtx, nullptr, &Demuxer::writeVideoFrame);

end:
    _aOutFile.close();
    _vOutFile.close();
    avcodec_free_context(&_aDecodeCtx);
    avcodec_free_context(&_vDecodeCtx);
    avformat_close_input(&_fmtCtx);
    av_frame_free(&_frame);
    av_packet_free(&pkt);
    av_freep(&_imgbuf[0]);
}

int Demuxer::initAudioInfo() {
    int ret = initDecoder(&_aStreamIdx, &_aDecodeCtx, AVMEDIA_TYPE_AUDIO);
    RET(initDecoder);

    _aOutFile.setFileName(_aOut->filename);
    if (!_aOutFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << _aOut->filename;
        return -1;
    }
    _aOut->sampleRate = _aDecodeCtx->sample_rate;
    _aOut->sampleFmt = _aDecodeCtx->sample_fmt;
    _aOut->chLayout = _aDecodeCtx->channel_layout;

    _sampleSize = av_get_bytes_per_sample(_aOut->sampleFmt);
    _sampleFrameSize = _sampleSize * _aDecodeCtx->channels;

    return 0;
}

int Demuxer::initVideoInfo() {
    int ret = initDecoder(&_vStreamIdx, &_vDecodeCtx, AVMEDIA_TYPE_VIDEO);
    RET(initDecoder);

    _vOutFile.setFileName(_vOut->filename);
    if (!_vOutFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << _vOut->filename;
        return -1;
    }
    _vOut->width = _vDecodeCtx->width;
    _vOut->height = _vDecodeCtx->height;
    _vOut->pixFmt = _vDecodeCtx->pix_fmt;
//    _vOut->fps = _vDecodeCtx->framerate.num;
    AVRational framerate = av_guess_frame_rate(_fmtCtx, _fmtCtx->streams[_vStreamIdx], nullptr);
    _vOut->fps = framerate.num / framerate.den;


    ret = av_image_alloc(_imgbuf, _imglinesize, _vOut->width, _vOut->height, _vOut->pixFmt, 1);
    RET(av_image_alloc);
    _imgsize = ret;

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

//    AVCodec *decoder = nullptr;
//    if (stream->codecpar->codec_id == AV_CODEC_ID_AAC) {
//        decoder = avcodec_find_decoder_by_name("libfdk_aac");
//    } else {
//        decoder = avcodec_find_decoder(stream->codecpar->codec_id);
//    }
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

int Demuxer::decode(AVCodecContext *decodeCtx, AVPacket *pkt, void (Demuxer::*func)()) {
    int ret = avcodec_send_packet(decodeCtx, pkt);
    RET(avcodec_send_packet);

    while (true) {
        ret = avcodec_receive_frame(decodeCtx, _frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }
        RET(avcodec_receive_frame);

//        if (decodeCtx->codec->type == AVMEDIA_TYPE_AUDIO) {
//            writeAudioFrame();
//        } else if (decodeCtx->codec->type == AVMEDIA_TYPE_VIDEO) {
//            writeVideoFrame();
//        }

        (this->*func)();
    }
}

void Demuxer::writeAudioFrame() {
    if (av_sample_fmt_is_planar(_aOut->sampleFmt)) {
        for (int si = 0; si < _frame->nb_samples; si++) {
            for (int ci = 0; ci < _aDecodeCtx->channels; ci++) {
                char *begin = (char *)(_frame->data[ci] + si * _sampleSize);
                _aOutFile.write(begin, _sampleSize);
            }
        }

    } else {
//        int v1 = _frame->linesize[0];
//        int v2 = _frame->nb_samples * av_get_bytes_per_sample(_aOut->sampleFmt) * _aDecodeCtx->channels;
//        if (v1 != v2) {
//            qDebug() << "_frame->linesize[0]" << v1 << "_frame->nb_samples" << v2;
//        }

//        _aOutFile.write((char *)_frame->data[0], _frame->linesize[0]);

        _aOutFile.write((char *)_frame->data[0], _frame->nb_samples * _sampleFrameSize);
    }
}

void Demuxer::writeVideoFrame() {
//    _vOutFile.write((char*)_frame->data[0], _frame->linesize[0] * _vOut->height);
//    _vOutFile.write((char*)_frame->data[1], _frame->linesize[1] * _vOut->height >> 1);
//    _vOutFile.write((char*)_frame->data[2], _frame->linesize[2] * _vOut->height >> 1);

//    int imageSize = av_image_get_buffer_size(_vOut->pixFmt, _vOut->width, _vOut->height, 1);
//    _vOutFile.write((char*)_frame->data[0], imageSize);

//    qDebug() << _frame->linesize[0]
//            << _frame->linesize[1]
//            << _frame->linesize[2]
//            << _vOut->width
//            << _vOut->height;

//    _vOutFile.write((char*)_frame->data[0], (_frame->linesize[0] - 48) * _vOut->height);
//    _vOutFile.write((char*)_frame->data[1], (_frame->linesize[1] - 24) * _vOut->height >> 1);
//    _vOutFile.write((char*)_frame->data[2], (_frame->linesize[2] - 24) * _vOut->height >> 1);


    av_image_copy(_imgbuf, _imglinesize,
                  (const uint8_t **)(_frame->data), _frame->linesize,
                  _vOut->pixFmt, _vOut->width, _vOut->height);
    _vOutFile.write((char *)_imgbuf[0], _imgsize);
}

