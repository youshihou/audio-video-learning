#include "videoplayer.h"

int VideoPlayer::initAudioInfo() {
    int ret = initDecoder(&_aStream, &_aDecodeCtx, AVMEDIA_TYPE_AUDIO);
    RET(initDecoder);

    ret = initSwr();
    RET(initSwr);

    ret = initSDL();
    RET(initSDL);

    return 0;
}

int VideoPlayer::initSwr() {
    _aSwrInSpec.samplefmt = _aDecodeCtx->sample_fmt;
    _aSwrInSpec.sampleRate = _aDecodeCtx->sample_rate;
    _aSwrInSpec.chLayout = _aDecodeCtx->channel_layout;
    _aSwrInSpec.chs = _aDecodeCtx->channels;

    _aSwrOutSpec.samplefmt = AV_SAMPLE_FMT_S16;
    _aSwrOutSpec.sampleRate = 44100;
    _aSwrOutSpec.chLayout = AV_CH_LAYOUT_STEREO;
    _aSwrOutSpec.chs = av_get_channel_layout_nb_channels(_aSwrOutSpec.chLayout);
    _aSwrOutSpec.bytesPerSampleFrame = _aSwrOutSpec.chs * av_get_bytes_per_sample(_aSwrOutSpec.samplefmt);

    _aSwrCtx = swr_alloc_set_opts(nullptr,
                                  _aSwrOutSpec.chLayout,
                                  _aSwrOutSpec.samplefmt,
                                  _aSwrOutSpec.sampleRate,
                                  _aSwrInSpec.chLayout,
                                  _aSwrInSpec.samplefmt,
                                  _aSwrInSpec.sampleRate,
                                  0, nullptr);
    if (!_aSwrCtx) {
        qDebug() << "swr_alloc_set_opts error";
        return -1;
    }

    int ret = swr_init(_aSwrCtx);
    RET(swr_init);

    _aSwrInFrame = av_frame_alloc();
    if (!_aSwrInFrame) {
        qDebug() << "av_frame_alloc error";
        return -1;
    }

    _aSwrOutFrame = av_frame_alloc();
    if (!_aSwrOutFrame) {
        qDebug() << "av_frame_alloc error";
        return -1;
    }

    ret = av_samples_alloc(_aSwrOutFrame->data,
                           _aSwrOutFrame->linesize,
                           _aSwrOutSpec.chs,
                           4096, _aSwrOutSpec.samplefmt, 1);
    RET(av_samples_alloc);

    return 0;
}

int VideoPlayer::initSDL() {
    SDL_AudioSpec spec;
    spec.freq = _aSwrOutSpec.sampleRate;
    spec.format = AUDIO_S16LSB;
    spec.channels = _aSwrOutSpec.chs;
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
    _aMutex.lock();
    _aPktList.push_back(pkt);
    _aMutex.signal();
    _aMutex.unlock();
}

void VideoPlayer::clearAudioPktList() {
    _aMutex.lock();
    for (AVPacket &pkt : _aPktList) {
        av_packet_unref(&pkt);
    }
    _aPktList.clear();
    _aMutex.unlock();
}

void VideoPlayer::freeAudio() {
    _aSwrOutIdx = 0;
    _aSwrOutSize = 0;

    clearAudioPktList();
    avcodec_free_context(&_aDecodeCtx);
    swr_free(&_aSwrCtx);
    av_frame_free(&_aSwrInFrame);
    if (_aSwrOutFrame) {
        av_freep(&_aSwrOutFrame->data[0]);
        av_frame_free(&_aSwrOutFrame);
    }

    SDL_PauseAudio(1);
    SDL_CloseAudio();
}

void VideoPlayer::SDLAudioCallbackFunc(void *userdata, Uint8 *stream, int len) {
    VideoPlayer *player = (VideoPlayer *)userdata;
    player->SDLAudioCallback(stream, len);
}

void VideoPlayer::SDLAudioCallback(Uint8 *stream, int len) {
    SDL_memset(stream, 0, len);

    while (len > 0) {
        if (_state == Stopped) { break; }

        if (_aSwrOutIdx >=  _aSwrOutSize) {
            _aSwrOutSize = decodeAudio();
            if (_aSwrOutSize <= 0) {
                memset(_aSwrOutFrame->data[0], 0, _aSwrOutSize = 1024);
            }
            _aSwrOutIdx = 0;
        }
        int fillLen = _aSwrOutSize - _aSwrOutIdx;
        fillLen = std::min(fillLen, len);
        int volumn = _mute ? 0 : ((_volumn * 1.0 / Max) * SDL_MIX_MAXVOLUME);
        SDL_MixAudio(stream, _aSwrOutFrame->data[0] + _aSwrOutIdx, fillLen, volumn);
        len -= fillLen;
        stream += fillLen;
        _aSwrOutIdx += fillLen;
    }
}

int VideoPlayer::decodeAudio() {
    _aMutex.lock();
//    while (_aPktList.empty()) {
//        _aMutex.wait();
//    }

    if (_aPktList.empty() || _state == Stopped) {
        _aMutex.unlock();
        return 0;
    }

    AVPacket pkt = _aPktList.front();
    _aPktList.pop_front();
    _aMutex.unlock();

    int ret = avcodec_send_packet(_aDecodeCtx, &pkt);
    av_packet_unref(&pkt);
    RET(avcodec_send_packet);

    ret = avcodec_receive_frame(_aDecodeCtx, _aSwrInFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return 0;
    } else RET(avcodec_receive_frame);

    int outSamples = av_rescale_rnd(_aSwrOutSpec.sampleRate,
                                    _aSwrInFrame->nb_samples,
                                    _aSwrOutSpec.sampleRate,
                                    AV_ROUND_UP);
    ret = swr_convert(_aSwrCtx,
                      _aSwrOutFrame->data, outSamples,
                      (const uint8_t **)_aSwrInFrame->data, _aSwrInFrame->nb_samples);
    RET(swr_convert);


    return ret * _aSwrOutSpec.bytesPerSampleFrame;
}
