#include "mainwindow.h"
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QPushButton *button = new QPushButton;
    button->setText("Close");
    button->setFixedSize(100, 44);
    button->setParent(this);
    connect(button, &QPushButton::clicked, this, &QMainWindow::close);
}

MainWindow::~MainWindow()
{
}

