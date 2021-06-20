#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <SDL2/SDL.h>
#include <list>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_produceButton_clicked();

private:
    Ui::MainWindow *ui;

    std::list<QString> *_list;
    int _index = 0;

    SDL_mutex *_mutex = nullptr;
    // 消费者等待，生产者唤醒
    SDL_cond *_cond1 = nullptr;
    // 生产者等待，消费者唤醒
    SDL_cond *_cond2 = nullptr;

    void produce(QString name);
    void consume(QString name);
};
#endif // MAINWINDOW_H
