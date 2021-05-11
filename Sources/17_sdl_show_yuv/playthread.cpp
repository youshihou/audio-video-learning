#include "playthread.h"

#include <SDL2/SDL.h>
#include <QDebug>


PlayThread::PlayThread(QObject *parent) : QThread(parent) {
    connect(this, &PlayThread::finished, this, &PlayThread::deleteLater);
}

PlayThread::~PlayThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();
    qDebug() << this << "deStruct";
}

void PlayThread::run() {

}
