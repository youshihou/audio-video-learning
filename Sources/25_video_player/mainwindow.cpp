#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _player = new VideoPlayer();
    connect(_player, &VideoPlayer::stateChanged, this, &MainWindow::onPlayerstateChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onPlayerstateChanged(VideoPlayer *player) {
    VideoPlayer::State state = player->getState();
    if (state == VideoPlayer::Playing) {
        ui->playButton->setText("pause");
    } else {
        ui->playButton->setText("play");
    }

    if (state == VideoPlayer::Stopped) {
        ui->playButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
        ui->currentSlider->setEnabled(false);
        ui->volumnSlider->setEnabled(false);
        ui->muteButton->setEnabled(false);

        ui->durationLabel->setText("00:00:00");

        ui->currentSlider->setValue(0);
        ui->volumnSlider->setValue(ui->volumnSlider->maximum());
    } else {
        ui->playButton->setEnabled(true);
        ui->stopButton->setEnabled(true);
        ui->currentSlider->setEnabled(true);
        ui->volumnSlider->setEnabled(true);
        ui->muteButton->setEnabled(true);
    }
}

void MainWindow::on_stopButton_clicked()
{
//    int count = ui->playWidget->count();
//    int idx = ui->playWidget->currentIndex();
//    ui->playWidget->setCurrentIndex(++idx % count);
    _player->stop();
}

void MainWindow::on_openfileButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(nullptr,
                                                    "Select Media File",
                                                    "/Users/ankui/Desktop/zzz/demux",
                                                    "Video File (*.mp4 *.avi *.mkv);;Audio File (*.mp3 *.aac);;All File (*.*)");
    qDebug() << filename;
    if (filename.isEmpty()) { return; }

//    QStringList filenames = QFileDialog::getOpenFileNames(nullptr,
//                                                          "Select Media File",
//                                                          "/Users/ankui/Desktop/zzz/demux",
//                                                          "Video File (*.mp4 *.avi *.mkv);;Audio File (*.mp3 *.aac);;All File (*.*)");
//    foreach (QString filename, filenames) {
//        qDebug() << filename;
//    }

    _player->setFilename(filename.toUtf8().data());
    _player->play();
}

void MainWindow::on_currentSlider_valueChanged(int value)
{
    ui->currentLabel->setText(QString("%1").arg(value));
}

void MainWindow::on_volumnSlider_valueChanged(int value)
{
    ui->volumnLabel->setText(QString("%1").arg(value));
}

void MainWindow::on_playButton_clicked()
{
    VideoPlayer::State state = _player->getState();
    if (state == VideoPlayer::Playing) {
        _player->pause();
    } else {
        _player->play();
    }
}
