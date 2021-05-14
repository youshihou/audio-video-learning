#include "mainwindow.h"

#include <QApplication>

#include "ffmpegs.h"

int main(int argc, char *argv[])
{

//    RawVideoFile in = {
//        "/Users/ankui/Desktop/zzz/yuv/out.yuv",
//        848, 464, AV_PIX_FMT_YUV420P,
//    };
//    RawVideoFile out = {
//        "/Users/ankui/Desktop/zzz/yuv/out2.rgb",
////        848, 464, AV_PIX_FMT_RGB24,
//        600, 600, AV_PIX_FMT_NV42,
//    };
//    FFmpegs::convertRawVideo(in, out);




    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
