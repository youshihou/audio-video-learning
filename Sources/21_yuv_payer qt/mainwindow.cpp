#include "mainwindow.h"
#include "ui_mainwindow.h"


static int yuvIdx = 0;
static const Yuv yuvs[] = {
    {
            "/Users/ankui/Desktop/zzz/yuv/out.yuv",
            848, 464,
            AV_PIX_FMT_YUV420P,
            30
    },
    {
            "/Users/ankui/Desktop/zzz/yuv/in.yuv",
            512, 512,
            AV_PIX_FMT_YUV420P,
            30
    },
    {
            "/Users/ankui/Desktop/zzz/yuv/2.yuv",
            464, 848,
            AV_PIX_FMT_YUV420P,
            30
    },
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(_player, &YuvPlayer::stateChanged, this, &MainWindow::onPlayerStateChanged);

    _player = new YuvPlayer(this);
    int w = 500;
    int h = 500;
    int x = (width() - w) >> 1;
    int y = (height() - h) >> 1;
    _player->setGeometry(x, y, w, h);
//    Yuv yuv = {
//        "/Users/ankui/Desktop/zzz/yuv/out.yuv",
//        848, 464,
//        AV_PIX_FMT_YUV420P,
//        30
//    };
//    _player->setYuv(yuv);

    Yuv yuv = yuvs[yuvIdx];
    _player->setYuv(yuv);
    _player->play();
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

void MainWindow::on_nextButton_clicked() {
    int count = sizeof (yuvs) / sizeof (Yuv);
    yuvIdx = ++yuvIdx % count;
    Yuv yuv = yuvs[yuvIdx];
    _player->stop();
    _player->setYuv(yuv);
    _player->play();
}

void MainWindow::onPlayerStateChanged() {
    if (_player->getState() == YuvPlayer::Playing) {
        ui->playButton->setText("pause");
    } else {
        ui->playButton->setText("play");
    }
}

