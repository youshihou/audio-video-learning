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
    _file.close();
    freeCurrentImage();
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


    _file.setFileName(yuv.filename);
    if (!_file.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << yuv.filename;
    }
}

YuvPlayer::State YuvPlayer::getState() {
    return _state;
}



void YuvPlayer::freeCurrentImage() {
    if (!_currentImage) { return; }
    free(_currentImage->bits());
    delete _currentImage;
    _currentImage = nullptr;
}


void YuvPlayer::timerEvent(Q_DECL_UNUSED QTimerEvent *event) {
    int imageSize = av_image_get_buffer_size(_yuv.pixelFormat, _yuv.width, _yuv.height, 1);
    char data[imageSize];
    if (_file.read(data, imageSize) > 0) {
        RawVideoFrame in = {
            data,
            _yuv.width, _yuv.height,
            _yuv.pixelFormat
        };
        RawVideoFrame out = {
            nullptr,
            _yuv.width, _yuv.height,
            AV_PIX_FMT_RGB24
        };
        FFmpegs::convertRawVideo(in, out);

        freeCurrentImage();
        _currentImage = new QImage((uchar *)out.pixels, out.width, out.height, QImage::Format_RGB888);
        update();
    } else {
        killTimer(_timerId);
    }
}

void YuvPlayer::paintEvent(Q_DECL_UNUSED QPaintEvent *event) {
    if (!_currentImage) { return; }

//    QPainter painter(this);
//    painter.drawImage(QPoint(0, 0), *_currentImage);

    QPainter(this).drawImage(QRect(0, 0, width(), height()), *_currentImage);
}
