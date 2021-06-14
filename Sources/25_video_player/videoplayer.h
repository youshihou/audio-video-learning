#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    typedef enum {
        Stopped = 0,
        Playing,
        Paused
    } State;

    explicit VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer();


    void play();
    void pause();
    void stop();
    bool isPlaying();

    State getState();
    void setFilename(const char *filename);
    int64_t getDuration();


signals:
    void stateChanged(VideoPlayer *player);
    void initFinished(VideoPlayer *player);
    void playFailed(VideoPlayer *player);


private:
    State _state = Stopped;
    const char *_filename;

    AVFormatContext *_fmtCtx = nullptr;
    AVCodecContext *_aDecodeCtx = nullptr, *_vDecodeCtx = nullptr;
    AVStream *_aStream = nullptr, *_vStream = nullptr;
    AVFrame *_aFrame = nullptr, *_vFrame = nullptr;

    int initAudioInfo();
    int initVideoInfo();
    int initDecoder(AVStream **stream, AVCodecContext** decodeCtx, AVMediaType type);


    void setState(State state);
    void readFile();

};

#endif // VIDEOPLAYER_H
