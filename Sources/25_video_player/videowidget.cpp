#include "videowidget.h"
#include <QPainter>
#include <QDebug>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background: black");
}

VideoWidget::~VideoWidget() {
    freeImage();
}


void VideoWidget::onPlayerFrameDecoded(VideoPlayer *player, uint8_t *data, VideoPlayer::VideoSwsSpec &spec) {
    if (player->getState() == VideoPlayer::Stopped) { return; }

    freeImage();

    if (data != nullptr) {
        _image = new QImage((uchar *)data,
                            spec.width, spec.height,
                            QImage::Format_RGB888);

        int w = width();
        int h = height();

        int dx = 0;
        int dy = 0;
        int dw = spec.width;
        int dh = spec.height;

        if (dw > w || dh > h) {
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

        _rect = QRect(dx, dy, dw, dh);
    }


    update();
}

void VideoWidget::onPlayerStateChanged(VideoPlayer* player) {
    if (player->getState() != VideoPlayer::Stopped) { return; }

    freeImage();
    update();
}

void VideoWidget::freeImage() {
    if (_image) {
        av_free(_image->bits());
        delete _image;
        _image = nullptr;
    }
}


void VideoWidget::paintEvent(QPaintEvent *event) {
    if (!_image) { return; }

    QPainter(this).drawImage(_rect, *_image);
}
