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
    _cond1 = SDL_CreateCond();
    _cond2 = SDL_CreateCond();

    // produce
    _list = new std::list<QString>();
    // consume
    consume("consume1");
    consume("consume2");
    consume("consume3");
    consume("consume4");

    // produce
    produce("produce1");
    produce("produce2");
    produce("produce3");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete _list;
    SDL_DestroyMutex(_mutex);
    SDL_DestroyCond(_cond1);
    SDL_DestroyCond(_cond2);
}


void MainWindow::on_produceButton_clicked()
{

}

void MainWindow::produce(QString name) {
    std::thread([this, name]() {
        SDL_LockMutex(_mutex);

        while (true) {
            qDebug() << name << "start produce";

            _list->push_back(QString("%1").arg(++_index));
            _list->push_back(QString("%1").arg(++_index));
            _list->push_back(QString("%1").arg(++_index));

            // 唤醒消费者
            SDL_CondSignal(_cond1);
            // 等待被消费者唤醒
            SDL_CondWait(_cond2, _mutex);
        }

        SDL_UnlockMutex(_mutex);
    }).detach();
}

void MainWindow::consume(QString name) {
    std::thread([this, name]() {
        SDL_LockMutex(_mutex);

        while (true) {
            qDebug() << name << "start consume";

            while (!_list->empty()) {
                qDebug() << _list->front();
                _list->pop_front();

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            // 唤醒生产者
            SDL_CondSignal(_cond2);
            // 等待被生产者唤醒
            SDL_CondWait(_cond1, _mutex);
        }

        SDL_UnlockMutex(_mutex);
    }).detach();
}
