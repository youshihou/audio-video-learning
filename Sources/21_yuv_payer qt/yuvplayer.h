#ifndef YUVPLAYER_H
#define YUVPLAYER_H

#include <QWidget>
#include <QFile>

extern "C" {
#include <libavutil/avutil.h>
}



typedef struct {
    const char *filename;
    int width;
    int height;
    AVPixelFormat pixelFormat;
    int fps;
} Yuv;


class YuvPlayer : public QWidget
{
    Q_OBJECT
public:
    explicit YuvPlayer(QWidget *parent = nullptr);
    ~YuvPlayer();

    typedef enum {
        Stopped = 0,
        Playing,
        Paused,
        Finished
    } State;

    void play();
    void pause();
    void stop();
    bool isPlaying();
    void setYuv(Yuv &yuv);

    State getState();

signals:
    void stateChanged();

private:
    Yuv _yuv;
    int _timerId = 0;
    int _interval = 0;
    int _imageSize = 0;
    State _state = Stopped;
    QImage *_currentImage = nullptr;
    QFile *_file = nullptr;
    QRect _dstRect;

    void setState(State state);

    void stopTimer();
    void closeFile();
    void freeCurrentImage();

    void timerEvent(QTimerEvent* event);
    void paintEvent(QPaintEvent *event);
};

#endif // YUVPLAYER_H
