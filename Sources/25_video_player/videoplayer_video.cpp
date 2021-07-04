#include "videoplayer.h"
#include <thread>
#include <QDebug>


extern "C" {
#include <libavutil/imgutils.h>
}


int VideoPlayer::initVideoInfo() {
    int ret = initDecoder(&_vStream, &_vDecodeCtx, AVMEDIA_TYPE_VIDEO);
    RET(initDecoder);

    ret = initSws();
    RET(initSws);

    return 0;
}

int VideoPlayer::initSws() {
    int inWidth = _vDecodeCtx->width;
    int inHeight = _vDecodeCtx->height;
    _vSwsOutSpec.width = inWidth >> 4 << 4;
    _vSwsOutSpec.height = inHeight >> 4 << 4;
    _vSwsOutSpec.pixFmt = AV_PIX_FMT_RGB24;
    _vSwsOutSpec.size = av_image_get_buffer_size(_vSwsOutSpec.pixFmt,
                                                 _vSwsOutSpec.width,
                                                 _vSwsOutSpec.height, 1);

    _vSwsCtx = sws_getContext(inWidth, inHeight, _vDecodeCtx->pix_fmt,
                              _vSwsOutSpec.width, _vSwsOutSpec.height, _vSwsOutSpec.pixFmt,
                              SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!_vSwsCtx) {
        qDebug() << "sws_getContext error";
        return -1;
    }

    _vSwsInFrame = av_frame_alloc();
    if (!_vSwsInFrame) {
        qDebug() << "av_frame_alloc error";
        return -1;
    }

    _vSwsOutFrame = av_frame_alloc();
    if (!_vSwsOutFrame) {
        qDebug() << "av_frame_alloc error";
        return -1;
    }

    int ret = av_image_alloc(_vSwsOutFrame->data,
                             _vSwsOutFrame->linesize,
                             _vSwsOutSpec.width,
                             _vSwsOutSpec.height,
                             _vSwsOutSpec.pixFmt,
                             1);
    RET(av_image_alloc);

    return 0;
}

void VideoPlayer::addVideoPkt(AVPacket &pkt) {
    _vMutex.lock();
    _vPktList.push_back(pkt);
    _vMutex.signal();
    _vMutex.unlock();
}

void VideoPlayer::clearVideoPktList() {
    _vMutex.lock();
    for (AVPacket &pkt : _vPktList) {
        av_packet_unref(&pkt);
    }
    _vPktList.clear();
    _vMutex.unlock();
}

void VideoPlayer::freeVideo() {
    clearVideoPktList();
    avcodec_free_context(&_vDecodeCtx);
    av_frame_free(&_vSwsInFrame);
    if (_vSwsOutFrame) {
        av_freep(&_vSwsOutFrame->data[0]);
        av_frame_free(&_vSwsOutFrame);
    }
    sws_freeContext(_vSwsCtx);
    _vSwsCtx = nullptr;
    _vStream = nullptr;
    _vTime = 0;
    _vCanFree = false;
}

void VideoPlayer::decodeVideo() {
    while (true) {
        if (_state == Stopped) {
            _vCanFree = true;
            break;
        }

        _vMutex.lock();
        if (_vPktList.empty()) {
            _vMutex.unlock();
            continue;
        }

        AVPacket pkt = _vPktList.front();
        _vPktList.pop_front();
        _vMutex.unlock();

        int ret = avcodec_send_packet(_vDecodeCtx, &pkt);

        if (pkt.dts != AV_NOPTS_VALUE) {
            _vTime = av_q2d(_vStream->time_base) * pkt.dts;
        }

        av_packet_unref(&pkt);
        CONTINUE(avcodec_send_packet);

        while (true) {
            ret = avcodec_receive_frame(_vDecodeCtx, _vSwsInFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else BREAK(avcodec_receive_frame);

            sws_scale(_vSwsCtx,
                      _vSwsInFrame->data, _vSwsInFrame->linesize,
                      0, _vDecodeCtx->height,
                      _vSwsOutFrame->data, _vSwsOutFrame->linesize);

            if (_hasAudio) {
                while (_vTime > _aTime && _state == Playing) {
//                    SDL_Delay(5);
                }
            } else {

            }

            uint8_t *data = (uint8_t *)av_malloc(_vSwsOutSpec.size);
            memcpy(data, _vSwsOutFrame->data[0], _vSwsOutSpec.size);

            emit frameDecoded(this, _vSwsOutFrame->data[0], _vSwsOutSpec);
        }
    }
}
