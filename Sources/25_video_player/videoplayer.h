#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QDebug>
#include <list>
#include "condmutex.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}



#define ERROR_BUFFER \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof(errbuf));

#define END(func) \
    if (ret < 0) { \
        ERROR_BUFFER; \
        qDebug() << #func << "error:" << ret << errbuf; \
        setState(Stopped); \
        emit playFailed(this); \
        goto end; \
    }

#define RET(func) \
    if (ret < 0) { \
        ERROR_BUFFER; \
        qDebug() << #func << "error" << errbuf; \
        return ret; \
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
    /****************** Audio *******************/
    AVCodecContext *_aDecodeCtx = nullptr;
    AVStream *_aStream = nullptr;
    AVFrame *_aFrame = nullptr;
    std::list<AVPacket> *_aPktList = nullptr;
    CondMutex *_aMutex = nullptr;

    int initAudioInfo();
    int initSDL();
    void addAudioPkt(AVPacket &pkt);
    void clearAudioPktList();
    static void SDLAudioCallbackFunc(void *userdata, Uint8 *stream, int len);
    void SDLAudioCallback(Uint8 *stream, int len);
    int decodeAudio();


    /****************** Video *******************/
    AVCodecContext *_vDecodeCtx = nullptr;
    AVStream *_vStream = nullptr;
    AVFrame *_vFrame = nullptr;
    std::list<AVPacket> *_vPktList = nullptr;
    CondMutex *_vMutex = nullptr;

    int initVideoInfo();
    void addVideoPkt(AVPacket &pkt);
    void clearVideoPktList();





    /****************** Other *******************/
    State _state = Stopped;
    const char *_filename;
    AVFormatContext *_fmtCtx = nullptr;


    int initDecoder(AVStream **stream, AVCodecContext** decodeCtx, AVMediaType type);
    void setState(State state);
    void readFile();

};

#endif // VIDEOPLAYER_H
