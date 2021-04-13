#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include <QThread>

class AudioThread : public QThread
{
    Q_OBJECT
private:
    void run();
    bool _stop = false;
public:
    explicit AudioThread(QObject *parent = nullptr);
    ~AudioThread();
    void setStop(bool stop);
signals:
    void timeChange(unsigned long long ms);
};

#endif // AUDIOTHREAD_H
