#include "playthread.h"

#include <SDL2/SDL.h>
#include <QDebug>


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


    END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    surface = SDL_LoadBMP("/Users/ankui/Desktop/zzz/yuv/in.bmp");
    END(!surface, SDL_LoadBMP);

    window = SDL_CreateWindow("SDL show BMP image",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              surface->w, surface->h,
                              SDL_WINDOW_SHOWN);
    END(!window, SDL_CreateWindow);


    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    END(!renderer, SDL_CreateRenderer);

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    END(!texture, SDL_CreateTextureFromSurface);

    END(SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);
    END(SDL_RenderClear(renderer), SDL_RenderClear);

    END(SDL_RenderCopy(renderer, texture, nullptr, nullptr), SDL_RenderCopy);

    SDL_RenderPresent(renderer);

 end:
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
