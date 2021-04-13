#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTime>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    onTimeChange(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onTimeChange(unsigned long long ms) {
    QTime time(0, 0, 0, 0);
    QString text = time.addMSecs(ms).toString("mm:ss.z");
    ui->timeLabel->setText(text.left(7));
}

void MainWindow::on_audioButton_clicked() {
    if (_audioThread) {
        _audioThread->requestInterruption();
        _audioThread = nullptr;
        ui->audioButton->setText("start record");
    } else {
        _audioThread = new AudioThread(this);
        _audioThread->start();
        connect(_audioThread, &AudioThread::timeChange, this, &MainWindow::onTimeChange);

        connect(_audioThread, &AudioThread::finished, [this]() {
            _audioThread = nullptr;
            ui->audioButton->setText("start record");
        });
        ui->audioButton->setText("end record");
    }
}
