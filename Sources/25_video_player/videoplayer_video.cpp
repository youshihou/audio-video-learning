#include "videoplayer.h"



int VideoPlayer::initVideoInfo() {
    int ret = initDecoder(&_vStream, &_vDecodeCtx, AVMEDIA_TYPE_VIDEO);
    RET(initDecoder);


    return 0;
}

void VideoPlayer::addVideoPkt(AVPacket &pkt) {
    _vMutex->lock();
    _vPktList->push_back(pkt);
    _vMutex->signal();
    _vMutex->unlock();
}

void VideoPlayer::clearVideoPktList() {
    _vMutex->lock();
    for (AVPacket &pkt : *_vPktList) {
        av_packet_unref(&pkt);
    }
    _vMutex->unlock();
}

void VideoPlayer::freeVideo() {

}
