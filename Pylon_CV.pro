#-------------------------------------------------
#
# Project created by QtCreator 2018-06-17T21:27:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Pylon_CV
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /opt/pylon5/include
INCLUDEPATH += /usr/local/include/opencv

LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui
QMAKE_LFLAGS += -Wl,--enable-new-dtags -Wl,-rpath,/opt/pylon5/lib64
LIBS += -L/opt/pylon5/lib64
LIBS += -Wl,-E
LIBS += -lGCBase_gcc_v3_0_Basler_pylon_v5_0
LIBS += -lGenApi_gcc_v3_0_Basler_pylon_v5_0
LIBS += -lgxapi
LIBS += -lpylonbase
LIBS += -lpylonutility
LIBS += -luxapi
LIBS += -lpylon_TL_usb
LIBS += -lMathParser_gcc_v3_0_Basler_pylon_v5_0
LIBS += -lNodeMapData_gcc_v3_0_Basler_pylon_v5_0
LIBS += -llog4cpp_gcc_v3_0_Basler_pylon_v5_0
LIBS += -lLog_gcc_v3_0_Basler_pylon_v5_0
LIBS += -lXmlParser_gcc_v3_0_Basler_pylon_v5_0

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    framegrabber.cpp \
    labeldisplayer.cpp \
    rulercalibrator.cpp \
    measuretool.cpp

HEADERS += \
        mainwindow.h \
    framegrabber.h \
    labeldisplayer.h \
    rulercalibrator.h \
    measuretool.h

FORMS += \
        mainwindow.ui
