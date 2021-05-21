#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _player = new YuvPlayer(this);
    int w = 900;
    int h = 300;
    int x = (width() - w) >> 1;
    int y = (height() - h) >> 1;
    _player->setGeometry(x, y, w, h);
    Yuv yuv = {
        "/Users/ankui/Desktop/zzz/yuv/out.yuv",
        848, 464,
        AV_PIX_FMT_YUV420P,
        30
    };
    _player->setYuv(yuv);

    connect(_player, &YuvPlayer::stateChanged, this, &MainWindow::onPlayerStateChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_playButton_clicked() {    
    if (_player->isPlaying()) {
        _player->pause();
//        ui->playButton->setText("play");
    } else {
        _player->play();
//        ui->playButton->setText("pause");
    }
}

void MainWindow::on_stopButton_clicked() {
    _player->stop();
}


void MainWindow::onPlayerStateChanged() {
    if (_player->getState() == YuvPlayer::Playing) {
        ui->playButton->setText("pause");
    } else {
        ui->playButton->setText("play");
    }
}
