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
#include <libswscale/swscale.h>
}



#define ERROR_BUFFER \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof(errbuf));

#define CODE(func, code) \
    if (ret < 0) { \
        ERROR_BUFFER; \
        qDebug() << #func << "error:" << ret << errbuf; \
        code; \
    }


#define END(func) CODE(func, fatalError(); return;)
#define RET(func) CODE(func, return ret;)
#define CONTINUE(func) CODE(func, continue;)
#define BREAK(func) CODE(func, break;)





class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    explicit VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer();

    typedef enum {
        Stopped = 0,
        Playing,
        Paused
    } State;

    typedef enum {
        Min = 0,
        Max = 100
    } Volumn;


    typedef struct {
        int width;
        int height;
        AVPixelFormat pixFmt;
        int size;
    } VideoSwsSpec;

    void play();
    void pause();
    void stop();
    bool isPlaying();

    State getState();
    void setFilename(QString &filename);
    int getDuration();
    int getTime();
    void setTime(int seekTime);
    void setVolumn(int volumn);
    int getVolumn();
    void setMute(bool mute);
    bool isMute();

signals:
    void stateChanged(VideoPlayer *player);
    void timeChanged(VideoPlayer *player);
    void initFinished(VideoPlayer *player);
    void playFailed(VideoPlayer *player);
    void frameDecoded(VideoPlayer *player, uint8_t *data, VideoSwsSpec &spec);


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
    AVFrame *_aSwrInFrame = nullptr;
    AVFrame *_aSwrOutFrame = nullptr;
    AudioSwrSpec _aSwrInSpec;
    AudioSwrSpec _aSwrOutSpec;
    int _aSwrOutIdx = 0;
    int _aSwrOutSize = 0;
    double _aTime = 0;
    bool _aCanFree = false;
    bool _hasAudio = false;


    int initAudioInfo();
    int initSwr();
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
    std::list<AVPacket> _vPktList;
    CondMutex _vMutex;
    SwsContext *_vSwsCtx = nullptr;
    AVFrame *_vSwsInFrame = nullptr;
    AVFrame *_vSwsOutFrame = nullptr;
    VideoSwsSpec _vSwsOutSpec;
    double _vTime = 0;
    bool _vCanFree = false;
    bool _hasVideo = false;



    int initVideoInfo();
    int initSws();
    void addVideoPkt(AVPacket &pkt);
    void clearVideoPktList();
    void decodeVideo();




    /****************** Other *******************/
    State _state = Stopped;
    char _filename[512];
    AVFormatContext *_fmtCtx = nullptr;
    int _volumn = Max;
    bool _mute = false;
    bool _fmtCtxCanFree = false;
    int _seekTime = -1;


    int initDecoder(AVStream **stream, AVCodecContext** decodeCtx, AVMediaType type);
    void setState(State state);
    void readFile();
    void free();
    void freeAudio();
    void freeVideo();
    void fatalError();

};

#endif // VIDEOPLAYER_H
