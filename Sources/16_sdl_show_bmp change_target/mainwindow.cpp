#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "playthread.h"


#include <SDL2/SDL.h>
#include <QDebug>


#define END(judge, func) \
if (judge) { \
    qDebug() << #func << "error" << SDL_GetError(); \
    goto end; \
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}




SDL_Texture* createTexture(SDL_Renderer* renderer) {
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_TARGET, 50, 50);
    if (!texture) { return nullptr; }
    if (SDL_SetRenderTarget(renderer, texture)) { return nullptr; }
//    if (SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_TRANSPARENT)) { return nullptr; }
//    if (SDL_RenderClear(renderer)) { return nullptr; }
    if (SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE)) { return nullptr; }
    SDL_Rect rect = {0, 0, 50, 50};
    if (SDL_RenderDrawRect(renderer, &rect)) { return nullptr; }
    if (SDL_RenderDrawLine(renderer, 0, 0, 50, 50)) { return nullptr; }
    if (SDL_RenderDrawLine(renderer, 50, 0, 0, 50)) { return nullptr; }

    return texture;
}


void showClick(SDL_Event &event, SDL_Renderer *renderer, SDL_Texture *texture) {
    SDL_MouseButtonEvent btn = event.button;
//    int x = btn.x;
//    int y = btn.y;
    int w = 0;
    int h = 0;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &w, &h)) { return; }
    int x = btn.x -  (w >> 1);
    int y = btn.y - (h >> 1);
    SDL_Rect dstRect = {x, y, w, h};

    if (SDL_RenderClear(renderer)) { return; }

    if (SDL_RenderCopy(renderer, texture, nullptr, &dstRect)) { return; }

//    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
//    SDL_RenderDrawRect(renderer, &dstRect);
    SDL_RenderPresent(renderer);
}

void sdl_show_bmp() {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    SDL_Rect dstRect = {100, 100, 50, 50};

    END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);


    window = SDL_CreateWindow("SDL modify render target",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              500, 500,
                              SDL_WINDOW_SHOWN);
    END(!window, SDL_CreateWindow);


    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    END(!renderer, SDL_CreateRenderer);

    texture = createTexture(renderer);
    END(!texture, createTexture);

    END(SDL_SetRenderTarget(renderer, nullptr), SDL_SetRenderTarget);

    END(SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);
    END(SDL_RenderClear(renderer), SDL_RenderClear);

    END(SDL_RenderCopy(renderer, texture, nullptr, &dstRect), SDL_RenderCopy);

    SDL_RenderPresent(renderer);

//    while (!isInterruptionRequested()) {
    while (1) {
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
                goto end;
            break;
            case SDL_MOUSEBUTTONUP:
            showClick(event, renderer, texture);
            break;
        }
    }

 end:
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


void MainWindow::on_playButton_clicked() {
//    PlayThread *thread = new PlayThread(this);
//    thread->start();

    sdl_show_bmp();
}



