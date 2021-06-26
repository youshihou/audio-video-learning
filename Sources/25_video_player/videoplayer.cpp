#include "videoplayer.h"
#include <thread>
#include <QThread>

extern "C" {
#include <libavutil/imgutils.h>
}




VideoPlayer::VideoPlayer(QObject *parent) : QObject(parent) {
    if (SDL_Init(SDL_INIT_AUDIO)) {
        qDebug() << "SDL_Init error" << SDL_GetError();
        emit playFailed(this);
        return;
    }
}

VideoPlayer::~VideoPlayer() {
    free();

    SDL_Quit();
}


#pragma mark - public method
void VideoPlayer::play() {
    if (_state == Playing) { return; }

    if (_state == Stopped) {
        // read file
        std::thread([this]() {
           readFile();
        }).detach();
    }

    setState(Playing);
}

void VideoPlayer::pause() {
    if (_state != Playing) { return; }

    setState(Paused);
}

void VideoPlayer::stop() {
    if (_state == Stopped) { return; }


    setState(Stopped);

    free();
}

bool VideoPlayer::isPlaying() {
    return _state == Playing;
}

VideoPlayer::State VideoPlayer::getState() {
    return _state;
}

void VideoPlayer::setFilename(QString &filename) {
    const char *name = filename.toUtf8().data();
    memcpy(_filename, name, strlen(name) + 1);
}

int64_t VideoPlayer::getDuration() {
    return _fmtCtx ? _fmtCtx->duration : 0;
}

void VideoPlayer::setVolumn(int volumn) {
    _volumn = volumn;
}

int VideoPlayer::getVolumn() {
    return _volumn;
}

void VideoPlayer::setMute(bool mute) {
    _mute = mute;
}

bool VideoPlayer::isMute() {
    return _mute;
}

#pragma mark - private method
int VideoPlayer::initDecoder(AVStream** stream, AVCodecContext** decodeCtx, AVMediaType type) {
    int ret = av_find_best_stream(_fmtCtx, type, -1, -1, nullptr, 0);
    RET(av_find_best_stream);

    int streamIdx = ret;
    *stream = _fmtCtx->streams[streamIdx];
    if (!*stream) {
        qDebug() << "stream is empty";
        return -1;
    }

    AVCodec *decoder = avcodec_find_decoder((*stream)->codecpar->codec_id);
    if (!decoder) {
        qDebug() << "decode not found" << (*stream)->codecpar->codec_id;
        return -1;
    }

    *decodeCtx = avcodec_alloc_context3(decoder);
    if (!*decodeCtx) {
        qDebug() << "avcodec_alloc_context3 error";
        return -1;
    }

    ret = avcodec_parameters_to_context(*decodeCtx, (*stream)->codecpar);
    RET(avcodec_parameters_to_context);

    ret = avcodec_open2(*decodeCtx, decoder, nullptr);
    RET(avcodec_open2);

    return 0;
}

void VideoPlayer::setState(State state) {
    if (state == _state) { return; }


    _state = state;
    emit stateChanged(this);
}

void VideoPlayer::readFile() {
    int ret = 0;
    ret = avformat_open_input(&_fmtCtx, _filename, nullptr, nullptr);
    END(avformat_open_input);

    ret = avformat_find_stream_info(_fmtCtx, nullptr);
    END(avformat_find_stream_info);

    av_dump_format(_fmtCtx, 0, _filename, 0);
    fflush(stderr);

    bool noAudio = initAudioInfo() < 0;
    bool noVideo = initVideoInfo() < 0;
    if (noAudio && noVideo) {
        fatalError();
        return;
    }

    emit initFinished(this);

    while (_state != Stopped) {
        AVPacket pkt;
        ret = av_read_frame(_fmtCtx, &pkt);
        if (ret == 0) {
            if (pkt.stream_index == _aStream->index) {
                addAudioPkt(pkt);
            } else if (pkt.stream_index == _vStream->index) {
                addVideoPkt(pkt);
            }
        } else if (ret == AVERROR_EOF) {
            qDebug() << "end of file....";
            break;
        } else {
            ERROR_BUFFER
            qDebug() << "av_read_frame error" << errbuf;
            continue;
        }
    }
}


void VideoPlayer::free() {
    avformat_close_input(&_fmtCtx);

    freeAudio();
    freeVideo();
}


void VideoPlayer::fatalError() {
    setState(Stopped);
    emit playFailed(this);
    free();
}
