#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <list>
#include <SDL2/SDL.h>

#include "condmutex.h"

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

//    SDL_mutex *_mutex = nullptr;
//    SDL_cond *_cond = nullptr;

    CondMutex *_mutex = nullptr;

    void produce(QString name);
    void consume(QString name);
};
#endif // MAINWINDOW_H
