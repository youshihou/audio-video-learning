#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>


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


void MainWindow::on_audioButton_clicked() {
    if (_audioThread) {
        _audioThread->requestInterruption();
        _audioThread = nullptr;
        ui->audioButton->setText("start record");
    } else {
        _audioThread = new AudioThread(this);
        _audioThread->start();
        connect(_audioThread, &AudioThread::finished, [this]() {
            _audioThread = nullptr;
            ui->audioButton->setText("start record");
        });
        ui->audioButton->setText("end record");
    }
}
