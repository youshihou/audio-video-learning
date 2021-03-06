QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    yuvplayer.cpp

HEADERS += \
    mainwindow.h \
    yuvplayer.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


mac: {
    SDL_HOME = /usr/local/Cellar/sdl2/2.0.14_1
    FFMPEG_HOME = /usr/local/ffmpeg
}

INCLUDEPATH += $${FFMPEG_HOME}/include

# 多个库-L后不能有空格！！！
LIBS += -L$${FFMPEG_HOME}/lib \
        -lavutil

INCLUDEPATH += $${SDL_HOME}/include

# 多个库-L后不能有空格！！！
LIBS += -L$${SDL_HOME}/lib \
        -lSDL2


