#include "playthread.h"

#include <SDL2/SDL.h>
#include <QDebug>


#define FILE_NAME "/Users/ankui/Desktop/out.wav"

typedef struct AudioBuffer {
    int len = 0;
    int pullLen = 0;
    Uint8 *data = nullptr;
} AudioBuffer;



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

void pull_audio_data(void *userdata, Uint8 *stream, int len) {
    SDL_memset(stream, 0, len);

    AudioBuffer *buffer = (AudioBuffer *) userdata;
    if (buffer->len <= 0) { return; }
    buffer->pullLen = (len > buffer->len) ? buffer->len : len;
    SDL_MixAudio(stream, buffer->data, buffer->pullLen, SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;
    buffer->len -= buffer->pullLen;
}

void PlayThread::run() {
    if (SDL_Init(SDL_INIT_AUDIO)) {
        qDebug() << "SDL_Init error" << SDL_GetError();
        return;
    }

    SDL_AudioSpec spec;
    Uint8 *data = nullptr;
    Uint32 len = 0;
    if (!SDL_LoadWAV(FILE_NAME, &spec, &data, &len)) {
        qDebug() << "SDL_LoadWAV error" << SDL_GetError();
        SDL_Quit();
        return;
    }
    spec.samples = 1024;
    spec.callback = pull_audio_data;

    AudioBuffer buffer;
    buffer.data = data;
    buffer.len = len;
    spec.userdata = &buffer;

    if (SDL_OpenAudio(&spec, nullptr)) {
        qDebug() << "SDL_OpenAudio error" << SDL_GetError();
        SDL_Quit();
        return;
    }

    int sampleSize = SDL_AUDIO_BITSIZE(spec.format);
    int bytesPerSample = (sampleSize * spec.channels) >> 3;

    SDL_PauseAudio(0);
    while (!isInterruptionRequested()) {
        if (buffer.len > 0) { continue; }
        if (buffer.len <= 0) {
            int samples = buffer.pullLen / bytesPerSample;
            int ms = samples * 1000 / spec.freq;
            SDL_Delay(ms);
            qDebug() << ms;
            break;
        }
    }

    SDL_FreeWAV(data);
    SDL_CloseAudio();
    SDL_Quit();
}
