#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    qRegisterMetaType<VideoPlayer::VideoSwsSpec>("VideoSwsSpec&");


    _player = new VideoPlayer();
    connect(_player, &VideoPlayer::stateChanged, this, &MainWindow::onPlayerStateChanged);
    connect(_player, &VideoPlayer::timeChanged, this, &MainWindow::onPlayerTimeChanged);
    connect(_player, &VideoPlayer::initFinished, this, &MainWindow::onPlayerInitFinished);
    connect(_player, &VideoPlayer::playFailed, this, &MainWindow::onPlayerPlayFailed);

    connect(_player, &VideoPlayer::frameDecoded, ui->videoWidget, &VideoWidget::onPlayerFrameDecoded);
    connect(_player, &VideoPlayer::stateChanged, ui->videoWidget, &VideoWidget::onPlayerStateChanged);


    ui->volumnSlider->setRange(VideoPlayer::Volumn::Min, VideoPlayer::Volumn::Max);
    ui->volumnSlider->setValue(ui->volumnSlider->maximum());
}

MainWindow::~MainWindow()
{
    delete ui;
    delete _player;
}

#pragma mark - private method
void MainWindow::onPlayerPlayFailed(VideoPlayer *player) {
    QMessageBox::critical(nullptr, "Tips", "Play Failed!");
}

void MainWindow::onPlayerInitFinished(VideoPlayer *player) {
    int microSeconds = player->getDuration();
    ui->currentSlider->setRange(0, microSeconds);

    ui->durationLabel->setText(getTimeText(microSeconds));
}

void MainWindow::onPlayerTimeChanged(VideoPlayer( *player)) {
    ui->currentSlider->setValue(player->getCurrent());
}

void MainWindow::onPlayerStateChanged(VideoPlayer *player) {
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

        ui->durationLabel->setText(getTimeText(0));
        ui->currentSlider->setValue(0);

        ui->playWidget->setCurrentWidget(ui->openfilePage);
    } else {
        ui->playButton->setEnabled(true);
        ui->stopButton->setEnabled(true);
        ui->currentSlider->setEnabled(true);
        ui->volumnSlider->setEnabled(true);
        ui->muteButton->setEnabled(true);

        ui->playWidget->setCurrentWidget(ui->videoPage);
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
                                                    "/Users/ankui/Desktop/zzz/player",
                                                    "Video File (*.mp4 *.avi *.mkv);;Audio File (*.mp3 *.aac);;All File (*.*)");
    qDebug() << "open file:" << filename;
    if (filename.isEmpty()) { return; }

//    QStringList filenames = QFileDialog::getOpenFileNames(nullptr,
//                                                          "Select Media File",
//                                                          "/Users/ankui/Desktop/zzz/demux",
//                                                          "Video File (*.mp4 *.avi *.mkv);;Audio File (*.mp3 *.aac);;All File (*.*)");
//    foreach (QString filename, filenames) {
//        qDebug() << filename;
//    }

    _player->setFilename(filename);
    _player->play();
}

void MainWindow::on_currentSlider_valueChanged(int value)
{
    ui->currentLabel->setText(getTimeText(value));
}

void MainWindow::on_volumnSlider_valueChanged(int value)
{
    ui->volumnLabel->setText(QString("%1").arg(value));
    _player->setVolumn(value);
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

void MainWindow::on_muteButton_clicked()
{
    if (_player->isMute()) {
        _player->setMute(false);
        ui->muteButton->setText("mute");
    } else {
        _player->setMute(true);
        ui->muteButton->setText("nomute");
    }
}

QString MainWindow::getTimeText(int value) {
//    int64_t seconds = value / 1000000;
//    int h = seconds / 3600;
////    int m = (seconds % 3600) / 60;
//    int m = (seconds / 60) % 60;
//    int s = seconds % 60;

    int h = value / 3600;
//    int m = (value % 3600) / 60;
    int m = (value / 60) % 60;
    int s = value % 60;

//    QString qh = QString("0%1").arg(h).right(2);
//    QString qm = QString("0%1").arg(m).right(2);
//    QString qs = QString("0%1").arg(s).right(2);
//    return QString("%1:%2:%3").arg(qh).arg(qm).arg(qs);

    QLatin1Char fill = QLatin1Char('0');
    return QString("%1:%2:%3")
            .arg(h, 2, 10, fill)
            .arg(m, 2, 10, fill)
            .arg(s, 2, 10, fill);
}
