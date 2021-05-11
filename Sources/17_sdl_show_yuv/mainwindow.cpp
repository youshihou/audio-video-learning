#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "playthread.h"

#include <SDL2/SDL.h>
#include <QDebug>
#include <QFile>

#define END(judge, func) \
if (judge) { \
    qDebug() << #func << "error" << SDL_GetError(); \
    goto end; \
}

#define FILENAME  "/Users/ankui/Desktop/zzz/yuv/in.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV
#define IMAGE_W 512
#define IMAGE_H 512


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



void sdl_show_yuv() {
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;

    QFile file(FILENAME);


    END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    window = SDL_CreateWindow("SDL show YUV image",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              IMAGE_W, IMAGE_H,
                              SDL_WINDOW_SHOWN);
    END(!window, SDL_CreateWindow);


    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
    }
    END(!renderer, SDL_CreateRenderer);

    texture = SDL_CreateTexture(renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_STREAMING, IMAGE_W, IMAGE_H);
    END(!texture, SDL_CreateTexture);

    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "file open erroe" << FILENAME;
        goto end;
    }
    END(SDL_UpdateTexture(texture, nullptr, file.readAll().constData(), IMAGE_W), SDL_UpdateTexture);

    END(SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);
    END(SDL_RenderClear(renderer), SDL_RenderClear);

    END(SDL_RenderCopy(renderer, texture, nullptr, nullptr), SDL_RenderCopy);

    SDL_RenderPresent(renderer);

    while (true) {
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
            goto end;
            break;
        }
    }

 end:
    file.close();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void MainWindow::on_playButton_clicked() {
    sdl_show_yuv();
}
