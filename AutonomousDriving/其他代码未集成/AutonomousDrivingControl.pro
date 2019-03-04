
QT += opengl

INCLUDEPATH += ../Support/Interface \
               ../Support/Interface/BaseLib \
               ../Support/Interface/Core \
               ../Support/Interface/HighAccuracyDataReader\
               ../Support/lcm-1.0.0 \
               ../Support/PublishGpsData \
               ../Support/Radar \
               ../InterProcessComm/include \
               ../Support/ibeodata


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    naviwidget.cpp \
    CmdLineInterpret.cpp \
    SerialPortWin32.cpp \
    gpscontroler.cpp \
    gpswidget.cpp \
    commontools.cpp \
    navidata.cpp \
    navimapwidget.cpp \
    navidrivingwidget.cpp \
    recvoutsidemsg.cpp \
    recvLcmMsg.cpp \
    camControl.cpp \
    imgdisplaywidget.cpp \
    VciCanDrive.cpp \
    cancontrol.cpp \
    canwidget.cpp \
    movementcontrol.cpp \
    showcaminfo.cpp

HEADERS += \
    mainwindow.h \
    naviwidget.h \
    CmdLineInterpret.h \
    SerialPortWin32.h \
    gpscontroler.h \
    gpswidget.h \
    commondebug.h \
    commontools.h \
    navidata.h \
    navimapwidget.h \
    navidrivingwidget.h \
    recvoutsidemsg.h \
    recvLcmMsg.h \
    radardata.h \
    camControl.h \
    imgdisplaywidget.h \
    VciCanDrive.h \
    cancontrol.h \
    canwidget.h \
    movementcontrol.h \
    showcaminfo.h

LIBS += -L../Support/HighAccuracyDataReader \
           -lHighAccuracyDataReader \
          ../Support/lcm-1.0.0/lib/lcm.lib \
          ../InterProcessComm/Debug/InterProcessComm.lib

FORMS += \
    gpswidget.ui \
    canwidget.ui

RESOURCES += \
    image.qrc
