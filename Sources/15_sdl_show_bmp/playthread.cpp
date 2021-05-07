#include "playthread.h"

#include <SDL2/SDL.h>
#include <QDebug>
#include <QFile>


#define END(judge, func) \
if (judge) { \
    qDebug() << #func << "error" << SDL_GetError(); \
    goto end; \
}


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
    SDL_Surface *surface = nullptr;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;

//    if (SDL_Init(SDL_INIT_VIDEO)) {
//        qDebug() << "SDL_Init error" << SDL_GetError();
//        goto end;
//    }
    END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    surface = SDL_LoadBMP("/Users/ankui/Desktop/zzz/yuv/in.bmp");
//    if (!surface) {
//        qDebug() << "SDL_LoadBMP error" << SDL_GetError();
//        goto end;
//    }
    END(!surface, SDL_LoadBMP);

    window = SDL_CreateWindow("SDL show BMP image",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              surface->w, surface->h,
                              SDL_WINDOW_SHOWN);
//    if (!window) {
//        qDebug() << "SDL_CreateWindow error" << SDL_GetError();
//        goto end;
//    }
    END(!window, SDL_CreateWindow);


    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, - 1, 0);
    }
    END(!renderer, SDL_CreateRenderer);

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    END(!texture, SDL_CreateTextureFromSurface);

    END(SDL_RenderCopy(renderer, texture, nullptr, nullptr), SDL_RenderCopy);

    SDL_RenderPresent(renderer);

    SDL_Delay(2000);

 end:
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
