#include "yuvplayer.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}


#define RET(judge, func) \
if (judge) { \
    qDebug() << #func << "error" << SDL_GetError(); \
    return; \
}



static const std::map<AVPixelFormat, SDL_PixelFormatEnum> PIXEL_FORMAT_MAP = {
    {AV_PIX_FMT_YUV420P, SDL_PIXELFORMAT_IYUV},
    {AV_PIX_FMT_YUYV422, SDL_PIXELFORMAT_YUY2},
    {AV_PIX_FMT_NONE, SDL_PIXELFORMAT_UNKNOWN}
};



YuvPlayer::YuvPlayer(QWidget *parent) : QWidget(parent) {
    _window = SDL_CreateWindowFrom((const void *)winId());
    RET(!_window, SDL_CreateWindowFrom);

    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer) {
        _renderer = SDL_CreateRenderer(_window, -1, 0);
    }
    RET(!_renderer, SDL_CreateRenderer);
}

YuvPlayer::~YuvPlayer() {
    _file.close();
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
}

void YuvPlayer::play() {
    _timerId = startTimer(1000 / _yuv.fps);
    _state = YuvPlayer::Playing;
}

void YuvPlayer::pause() {
    if (_timerId) {
        killTimer(_timerId);
    }
    _state = YuvPlayer::Paused;
}

void YuvPlayer::stop() {
    if (_timerId) {
        killTimer(_timerId);
    }
    _state = YuvPlayer::Stopped;
}

bool YuvPlayer::isPlaying() {
    return _state == YuvPlayer::Playing;
}

void YuvPlayer::setYuv(Yuv &yuv) {
    _yuv = yuv;

    _texture = SDL_CreateTexture(_renderer,
                                 PIXEL_FORMAT_MAP.find(yuv.pixelFormat)->second,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 yuv.width, yuv.height);
    RET(!_texture, SDL_CreateTexture);

    _file.setFileName(yuv.filename);
    if (!_file.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << yuv.filename;
    }
}

YuvPlayer::State YuvPlayer::getState() {
    return _state;
}


void YuvPlayer::timerEvent(QTimerEvent *event) {
    int imageSize = av_image_get_buffer_size(_yuv.pixelFormat, _yuv.width, _yuv.height, 1);
    char data[imageSize];
    if (_file.read(data, imageSize) > 0) {
        RET(SDL_UpdateTexture(_texture, nullptr, data, _yuv.width), SDL_UpdateTexture);

        RET(SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);
        RET(SDL_RenderClear(_renderer), SDL_RenderClear);

        RET(SDL_RenderCopy(_renderer, _texture, nullptr, nullptr), SDL_RenderCopy);

        SDL_RenderPresent(_renderer);
    } else {
        killTimer(_timerId);
    }
}
