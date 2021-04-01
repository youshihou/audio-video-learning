#include "playthread.h"

#include <SDL2/SDL.h>
#include <QDebug>
#include <QFile>


#define FILE_NAME "/Users/ankui/Desktop/out.pcm"
#define SAMPLE_RATE 44100
#define SAMPLE_SIZE 32
#define CHANNELS 2
#define SAMPLES 1024
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) / 8)
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)


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

int bufferLen;
char *bufferData;

void pull_audio_data(void *userdata, Uint8 *stream, int len) {
    SDL_memset(stream, 0, len);
    if (bufferLen <= 0) { return; }
    len = (len > bufferLen) ? bufferLen : len;
    SDL_MixAudio(stream, (Uint8 *) bufferData, len, SDL_MIX_MAXVOLUME);
    bufferData += len;
    bufferLen -= len;
}

void PlayThread::run() {
    if (SDL_Init(SDL_INIT_AUDIO)) {
        qDebug() << "SDL_Init error" << SDL_GetError();
        return;
    }

    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_F32LSB;
    spec.channels = CHANNELS;
    spec.samples = 1024;
    spec.callback = pull_audio_data;

    if (SDL_OpenAudio(&spec, nullptr)) {
        qDebug() << "SDL_OpenAudio error" << SDL_GetError();
        SDL_Quit();
        return;
    }

    QFile file(FILE_NAME);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << FILE_NAME;
        SDL_CloseAudio();
        SDL_Quit();
        return;
    }

    SDL_PauseAudio(0);
    char data[BUFFER_SIZE];
    while (!isInterruptionRequested()) {
        if (bufferLen > 0) { continue; }
        bufferLen = file.read(data, BUFFER_SIZE);
        if (bufferLen <= 0) { break; }
        bufferData = data;
    }

    file.close();
    SDL_CloseAudio();
    SDL_Quit();
}
