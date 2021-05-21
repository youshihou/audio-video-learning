#include "yuvplayer.h"
#include <QDebug>
#include <QPainter>
#include "ffmpegs.h"

extern "C" {
#include <libavutil/imgutils.h>
}


#define RET(judge, func) \
if (judge) { \
    qDebug() << #func << "error" << SDL_GetError(); \
    return; \
}



YuvPlayer::YuvPlayer(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background: black");
}

YuvPlayer::~YuvPlayer() {
    closeFile();
    freeCurrentImage();
    stopTimer();
}

void YuvPlayer::play() {
    if (_state == YuvPlayer::Playing) { return; }
    _timerId = startTimer(_interval);
    setState(YuvPlayer::Playing);
}

void YuvPlayer::pause() {
    if (_state != YuvPlayer::Playing) { return; }

    stopTimer();
    setState(YuvPlayer::Paused);
}

void YuvPlayer::stop() {
    if (_state == YuvPlayer::Stopped) { return; }

    stopTimer();
    freeCurrentImage();
    update();
    setState(YuvPlayer::Stopped);
}

bool YuvPlayer::isPlaying() {
    return _state == YuvPlayer::Playing;
}

void YuvPlayer::setYuv(Yuv &yuv) {
    _yuv = yuv;

    closeFile();

//    _file.setFileName(yuv.filename);
    _file = new QFile(yuv.filename);
    if (!_file->open(QFile::ReadOnly)) {
        qDebug() << "file open error" << yuv.filename;
    }

    _interval = 1000 / _yuv.fps;
    _imageSize = av_image_get_buffer_size(_yuv.pixelFormat, _yuv.width, _yuv.height, 1);


    int w = width();
    int h = height();

    int dx = 0;
    int dy = 0;
    int dw = _yuv.width;
    int dh = _yuv.height;

    if (dw > w || dh > h) {
        // 视频的宽高比大于播放器的宽高比
//        if ((dw / dh) > (w / h)) {
        if (dw * h > w * dh) {
            dh = w * dh / dw;
            dw = w;
        } else {
            dw = h * dw / dh;
            dh = h;
        }
    }

    dx = (w - dw) >> 1;
    dy = (h - dh) >> 1;

    _dstRect = QRect(dx, dy, dw, dh);

    qDebug() << _dstRect;
}

YuvPlayer::State YuvPlayer::getState() {
    return _state;
}



void YuvPlayer::setState(State state) {
    if (state == _state) { return; }

    if (state == YuvPlayer::Stopped || state == YuvPlayer::Finished) {
        _file->seek(0);
    }
    _state = state;
    emit stateChanged();
}

void YuvPlayer::stopTimer() {
    if (_timerId == 0) { return; }
    killTimer(_timerId);
    _timerId = 0;
}

void YuvPlayer::closeFile() {
    if (!_file) { return; }
    _file->close();
    delete  _file;
    _file = nullptr;
}

void YuvPlayer::freeCurrentImage() {
    if (!_currentImage) { return; }
    free(_currentImage->bits());
    delete _currentImage;
    _currentImage = nullptr;
}


void YuvPlayer::timerEvent(Q_DECL_UNUSED QTimerEvent *event) {
    char data[_imageSize];
    if (_file->read(data, _imageSize) == _imageSize) {
        RawVideoFrame in = {
            data,
            _yuv.width, _yuv.height,
            _yuv.pixelFormat
        };
        RawVideoFrame out = {
            nullptr,
            _yuv.width >> 4 << 4,
            _yuv.height >> 4 << 4,
//            _yuv.width, _yuv.height,
            AV_PIX_FMT_RGB24
        };
        FFmpegs::convertRawVideo(in, out);

        freeCurrentImage();
        _currentImage = new QImage((uchar *)out.pixels, out.width, out.height, QImage::Format_RGB888);
        update();
    } else {
        stopTimer();
        setState(YuvPlayer::Finished);
    }
}

void YuvPlayer::paintEvent(Q_DECL_UNUSED QPaintEvent *event) {
    if (!_currentImage) { return; }
    QPainter(this).drawImage(_dstRect, *_currentImage);
}
