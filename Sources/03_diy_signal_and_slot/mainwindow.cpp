#include "mainwindow.h"
#include "sender.h"
#include "receiver.h"

#include <QDebug>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QPushButton *b = new QPushButton;
    b->setText("Button");
    b->setFixedSize(100, 44);
    b->setParent(this);
//    connect(b, &QPushButton::clicked, this, &MainWindow::handleClick);
//    connect(b, &QPushButton::clicked, [](){
//        qDebug() << "Button Clicked";
//    });

    Sender *s = new Sender;
    Receiver *r = new Receiver;

    // error !!!
//    connect(b, &QPushButton::clicked, s, &Sender::exit1);

//    connect(s, &Sender::exit1, [](int n1, int n2) {
//       qDebug() << "Lamdba" << n1 << n2;
//    });
//    emit s->exit1(10, 20);
    connect(s, &Sender::exit1, r, &Receiver::handleExit1);
    connect(s, &Sender::exit2, r, &Receiver::handleExit2);

    connect(s, &Sender::exit1, s, &Sender::exit2);

    emit s->exit2(10, 20);
    qDebug() << emit s->exit1(10, 20);


    delete s;
    delete r;
}

MainWindow::~MainWindow()
{
}

void MainWindow::handleClick() {
    qDebug() << "MainWindow::handleClick";
}

