#include "videoplayer.h"

VideoPlayer::VideoPlayer(QObject *parent) : QObject(parent)
{

}

VideoPlayer::~VideoPlayer() {

}


#pragma mark - public method
void VideoPlayer::play() {
    if (_state == Playing) { return; }

    setState(Playing);
}

void VideoPlayer::pause() {
    if (_state != Playing) { return; }

    setState(Paused);
}

void VideoPlayer::stop() {
    if (_state == Stopped) { return; }


    setState(Stopped);
}

bool VideoPlayer::isPlaying() {
    return _state == Playing;
}

VideoPlayer::State VideoPlayer::getState() {
    return _state;
}

void VideoPlayer::setFilename(const char *filename) {
    _filename = filename;
}

#pragma mark - private method
void VideoPlayer::setState(State state) {
    if (state == _state) { return; }


    _state = state;
    emit stateChanged(this);
}
