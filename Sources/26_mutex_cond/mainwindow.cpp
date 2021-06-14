#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <thread>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _mutex = SDL_CreateMutex();
    _cond = SDL_CreateCond();

    // produce
    _list = new std::list<QString>();
    // consume
    consume("consume1");
    consume("consume2");
    consume("consume3");
    consume("consume4");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete _list;
    SDL_DestroyMutex(_mutex);
    SDL_DestroyCond(_cond);
}


void MainWindow::on_produceButton_clicked()
{
    produce("produce1");
}

void MainWindow::produce(QString name) {
    std::thread([this, name](){
        SDL_LockMutex(_mutex);

        qDebug() << name << "start produce";

        _list->push_back(QString("%1").arg(++_index));
        _list->push_back(QString("%1").arg(++_index));
        _list->push_back(QString("%1").arg(++_index));

        SDL_CondSignal(_cond);
        SDL_UnlockMutex(_mutex);
    }).detach();
}

void MainWindow::consume(QString name) {
    std::thread([this, name]() {
        SDL_LockMutex(_mutex);

        while (true) {
            qDebug() << name << "start lock";

            while (!_list->empty()) {
                qDebug() << _list->front();
                _list->pop_front();

                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            }

            qDebug() << name << "start waiting...";
            SDL_CondWait(_cond, _mutex);
        }

        SDL_UnlockMutex(_mutex);
    }).detach();
}
