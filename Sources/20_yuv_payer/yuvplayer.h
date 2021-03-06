#ifndef YUVPLAYER_H
#define YUVPLAYER_H

#include <QWidget>
#include <SDL2/SDL.h>
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

private:
    Yuv _yuv;
    SDL_Window *_window = nullptr;
    SDL_Renderer *_renderer = nullptr;
    SDL_Texture *_texture = nullptr;
    QFile _file;
    int _timerId = 0;
    State _state = Stopped;

    void timerEvent(QTimerEvent* event);
};

#endif // YUVPLAYER_H
