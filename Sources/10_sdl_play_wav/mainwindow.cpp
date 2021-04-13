#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <SDL2/SDL.h>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SDL_version version;
    SDL_VERSION(&version);
    qDebug() << version.major << version.minor << version.patch;
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_playButton_clicked() {
    if (_playThread) {
        _playThread->requestInterruption();
        _playThread = nullptr;
        ui->playButton->setText("start play");
    } else {
        _playThread = new PlayThread(this);
        _playThread->start();
        connect(_playThread, &PlayThread::finished, [this]() {
            _playThread = nullptr;
            ui->playButton->setText("start play");
        });
        ui->playButton->setText("stop play");
    }
}
