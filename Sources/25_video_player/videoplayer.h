#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>

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


signals:
    void stateChanged(VideoPlayer *player);


private:
    State _state = Stopped;
    const char *_filename;

    void setState(State state);

};

#endif // VIDEOPLAYER_H
