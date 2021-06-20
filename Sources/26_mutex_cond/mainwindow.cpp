#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <thread>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    _mutex = SDL_CreateMutex();
//    _cond = SDL_CreateCond();

    _mutex = new CondMutex();

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

//    SDL_DestroyMutex(_mutex);
//    SDL_DestroyCond(_cond);
    delete _mutex;
}


void MainWindow::on_produceButton_clicked() {
    produce("produce1");
    produce("produce2");
    produce("produce3");
}

void MainWindow::produce(QString name) {
    std::thread([this, name](){
//        SDL_LockMutex(_mutex);

        _mutex->lock();

        qDebug() << name << "start produce";

        _list->push_back(QString("%1").arg(++_index));
        _list->push_back(QString("%1").arg(++_index));
        _list->push_back(QString("%1").arg(++_index));

        // 唤醒等待_cond的线程
//        SDL_CondSignal(_cond);
//        SDL_UnlockMutex(_mutex);
        _mutex->signal();
        _mutex->unlock();
    }).detach();
}

void MainWindow::consume(QString name) {
    std::thread([this, name]() {
//        SDL_LockMutex(_mutex);

        _mutex->lock();

        while (true) {
            qDebug() << name << "get lock";

            while (!_list->empty()) {
                qDebug() << _list->front();
                _list->pop_front();

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            qDebug() << name << "start waiting...";

            // _list是空的，进入等待
            // 1.释放互斥锁
            // 2.等待条件_cond
            // 3.等到了条件_cond，加锁
//            SDL_CondWait(_cond, _mutex);
            _mutex->wait();
        }

//        SDL_UnlockMutex(_mutex);
        _mutex->unlock();
    }).detach();
}
