#include "receiver.h"
#include <QDebug>

Receiver::Receiver(QObject *parent) : QObject(parent)
{

}

int Receiver::handleExit1(int n1, int n2) {
    qDebug() << "Receiver::handleExit1" << n1;
    return n1;
}

void Receiver::handleExit2(int n1, int n2) {
    qDebug() << "Receiver::handleExit2";
}
