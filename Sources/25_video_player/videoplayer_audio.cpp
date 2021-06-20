#include "videoplayer.h"

int VideoPlayer::initAudioInfo() {
    int ret = initDecoder(&_aStream, &_aDecodeCtx, AVMEDIA_TYPE_AUDIO);
    RET(initDecoder);

    _aFrame = av_frame_alloc();
    if (!_aFrame) {
        qDebug() << "av_frame_alloc error";
        return -1;
    }

    ret = initSDL();
    RET(initSDL);

    return 0;
}

void VideoPlayer::SDLAudioCallbackFunc(void *userdata, Uint8 *stream, int len) {
    VideoPlayer *player = (VideoPlayer *)userdata;
    player->SDLAudioCallback(stream, len);
}


int VideoPlayer::initSDL() {
    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.format = AUDIO_S16LSB;
    spec.channels = 2;
    spec.samples = 512;
    spec.callback = SDLAudioCallbackFunc;
    spec.userdata = this;

    if (SDL_OpenAudio(&spec, nullptr)) {
        qDebug() << "SDL_OpenAudio error" << SDL_GetError();
        return -1;
    }


    SDL_PauseAudio(0);

    return 0;
}

void VideoPlayer::addAudioPkt(AVPacket &pkt) {
    _aMutex->lock();
    _aPktList->push_back(pkt);
    _aMutex->signal();
    _aMutex->unlock();
}

void VideoPlayer::clearAudioPktList() {
    _aMutex->lock();
    for (AVPacket &pkt : *_aPktList) {
        av_packet_unref(&pkt);
    }
    _aMutex->unlock();
}

void VideoPlayer::SDLAudioCallback(Uint8 *stream, int len) {
    while (len > 0) {
        int dataSize = decodeAudio();
        qDebug() << dataSize;
        if (dataSize <= 0) {

        } else {

        }
    }
}

int VideoPlayer::decodeAudio() {
    _aMutex->lock();
//    while (_aPktList->empty()) {
//        _aMutex->wait();
//    }

    if (_aPktList->empty()) {
        _aMutex->unlock();
        return 0;
    }

    AVPacket &pkt = _aPktList->front();
    _aPktList->pop_front();
    _aMutex->unlock();

    int ret = avcodec_send_packet(_aDecodeCtx, &pkt);
    av_packet_unref(&pkt);
    RET(avcodec_send_packet);

    ret = avcodec_receive_frame(_aDecodeCtx, _aFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return 0;
    } else RET(avcodec_receive_frame)

    qDebug() << _aFrame->sample_rate
             << _aFrame->channels
             << av_get_sample_fmt_name((AVSampleFormat)_aFrame->format);

    return _aFrame->nb_samples
            * _aFrame->channels
            * av_get_bytes_per_sample((AVSampleFormat)_aFrame->format);
}
