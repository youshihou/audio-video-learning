#include "mainwindow.h"

#include <QApplication>

//#include "ffmpegs.h"

extern "C" {
#include <libavdevice/avdevice.h>
}


int main(int argc, char *argv[])
{

//    WAVHeader header;
//    header.sampleRate = 44100;
//    header.bitsPerSample = 32;
//    header.numChannels = 2;
//    header.audioFormat = 3;
//    FFmpegs::pcm2wav(header, "/Users/ankui/Desktop/zzzzzzzz/out.pcm", "/Users/ankui/Desktop/zzzzzzzz/in.wav");


    avdevice_register_all();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
