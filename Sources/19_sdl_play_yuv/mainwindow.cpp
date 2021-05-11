#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

#define RET(judge, func) \
if (judge) { \
    qDebug() << #func << "error" << SDL_GetError(); \
    return; \
}


#define FILENAME  "/Users/ankui/Desktop/zzz/yuv/out.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV
#define IMAGE_W 848
#define IMAGE_H 464


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _widget = new QWidget(this);
    _widget->setGeometry(100, 50, IMAGE_W, IMAGE_H);

    RET(SDL_Init(SDL_INIT_VIDEO), SDL_Init);

    _window = SDL_CreateWindowFrom((const void *)_widget->winId());
    RET(!_window, SDL_CreateWindowFrom);

    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer) {
        _renderer = SDL_CreateRenderer(_window, -1, 0);
    }
    RET(!_renderer, SDL_CreateRenderer);

    _texture = SDL_CreateTexture(_renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_STREAMING, IMAGE_W, IMAGE_H);
    RET(!_texture, SDL_CreateTexture);

    _file.setFileName(FILENAME);
    if (!_file.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << FILENAME;
    }
}

MainWindow::~MainWindow()
{
    delete ui;

    _file.close();
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();
}

void MainWindow::on_playButton_clicked() {

    _timerId = startTimer(33);
}

void MainWindow::timerEvent(QTimerEvent *event) {
    int imageSize = IMAGE_W * IMAGE_H * 1.5;
    char data[imageSize];

    if (_file.read(data, imageSize) > 0) {
        RET(SDL_UpdateTexture(_texture, nullptr, data, IMAGE_W), SDL_UpdateTexture);

        RET(SDL_SetRenderDrawColor(_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE), SDL_SetRenderDrawColor);
        RET(SDL_RenderClear(_renderer), SDL_RenderClear);

        RET(SDL_RenderCopy(_renderer, _texture, nullptr, nullptr), SDL_RenderCopy);

        SDL_RenderPresent(_renderer);
    } else {
        killTimer(_timerId);
    }

}
