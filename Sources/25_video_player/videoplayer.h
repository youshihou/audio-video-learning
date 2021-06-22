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
#include <libswresample/swresample.h>
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
        free(); \
        return; \
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

    typedef enum {
        Min = 0,
        Max = 100
    } Volumn;


    explicit VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer();


    void play();
    void pause();
    void stop();
    bool isPlaying();

    State getState();
    void setFilename(const char *filename);
    int64_t getDuration();
    void setVolumn(int volumn);
    int getVolumn();
    void setMute(bool mute);
    bool isMute();

signals:
    void stateChanged(VideoPlayer *player);
    void initFinished(VideoPlayer *player);
    void playFailed(VideoPlayer *player);


private:
    /****************** Audio *******************/
    typedef struct {
        int sampleRate;
        AVSampleFormat samplefmt;
        int chLayout;
        int chs;
        int bytesPerSampleFrame;
    } AudioSwrSpec;

    AVCodecContext *_aDecodeCtx = nullptr;
    AVStream *_aStream = nullptr;
    std::list<AVPacket> _aPktList;
    CondMutex _aMutex;
    SwrContext *_aSwrCtx = nullptr;
    AudioSwrSpec _aSwrInSpec;
    AudioSwrSpec _aSwrOutSpec;
    AVFrame *_aSwrInFrame = nullptr;
    AVFrame *_aSwrOutFrame = nullptr;
    int _aSwrOutIdx = 0;
    int _aSwrOutSize = 0;
    int _volumn = Max;
    bool _mute = false;

    int initAudioInfo();
    int initSwr();
    int initSDL();
    void addAudioPkt(AVPacket &pkt);
    void clearAudioPktList();
    static void SDLAudioCallbackFunc(void *userdata, Uint8 *stream, int len);
    void SDLAudioCallback(Uint8 *stream, int len);
    int decodeAudio();
    void freeAudio();



    /****************** Video *******************/
    AVCodecContext *_vDecodeCtx = nullptr;
    AVStream *_vStream = nullptr;
    AVFrame *_vFrame = nullptr;
    std::list<AVPacket> _vPktList;
    CondMutex _vMutex;

    int initVideoInfo();
    void addVideoPkt(AVPacket &pkt);
    void clearVideoPktList();
    void freeVideo();




    /****************** Other *******************/
    State _state = Stopped;
    const char *_filename;
    AVFormatContext *_fmtCtx = nullptr;


    int initDecoder(AVStream **stream, AVCodecContext** decodeCtx, AVMediaType type);
    void setState(State state);
    void readFile();
    void free();


};

#endif // VIDEOPLAYER_H
