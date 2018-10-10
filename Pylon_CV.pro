#-------------------------------------------------
#
# Project created by QtCreator 2018-06-17T21:27:41
#
#-------------------------------------------------

QT       += core gui concurrent serialbus network mqtt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Pylon_CV
TEMPLATE = app

# Python 3.6 native library in ubuntu 18.04.1
INCLUDEPATH += /usr/include/python3.6m
LIBS += -L/usr/local/lib/python3.6 -lpython3.6m

# OpenCV 3.4.0, 3.4.2, 3.4.3 tested
INCLUDEPATH += /usr/local/include/opencv
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui

# Pylon 5.1.0 SDK
INCLUDEPATH += /opt/pylon5/include
QMAKE_LFLAGS += -Wl,--enable-new-dtags -Wl,-rpath,/opt/pylon5/lib64
LIBS += -L/opt/pylon5/lib64
LIBS += -Wl,-E
LIBS += -lGCBase_gcc_v3_1_Basler_pylon_v5_1
LIBS += -lGenApi_gcc_v3_1_Basler_pylon_v5_1
LIBS += -lgxapi
LIBS += -lpylonbase
LIBS += -lpylonutility
LIBS += -luxapi
LIBS += -lpylon_TL_usb
LIBS += -lMathParser_gcc_v3_1_Basler_pylon_v5_1
LIBS += -lNodeMapData_gcc_v3_1_Basler_pylon_v5_1
LIBS += -llog4cpp_gcc_v3_1_Basler_pylon_v5_1
LIBS += -lLog_gcc_v3_1_Basler_pylon_v5_1
LIBS += -lXmlParser_gcc_v3_1_Basler_pylon_v5_1

# libcurl, to get token from http
LIBS += -lcurl

# boost
#INCLUDEPATH += /usr/include/boost
#LIBS += -L/usr/include/boost -lboost_python -lboost_system

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    framegrabber.cpp \
    labeldisplayer.cpp \
    settingdialog.cpp \
    fourierdescriptor.cpp \
    fdtester.cpp \
    pyclassification.cpp \
    py_cv_wrapper/pycvconverter.cpp \
    calibratorform.cpp \
    measuretool.cpp \
    triggerform.cpp \
    communication/modbusclient.cpp \
    test/modbustest.cpp \
    communication/mqttclient.cpp \
    test/mqtttest.cpp

HEADERS += \
        mainwindow.h \
    framegrabber.h \
    labeldisplayer.h \
    settingdialog.h \
    fourierdescriptor.h \
    fdtester.h \
    pyclassification.h \
    py_cv_wrapper/pycvconverter.h \
    calibratorform.h \
    measuretool.h \
    triggerform.h \
    communication/modbusclient.h \
    test/modbustest.h \
    communication/mqttclient.h \
    test/mqtttest.h

FORMS += \
        mainwindow.ui \
    settingdialog.ui \
    calibratorform.ui \
    triggerform.ui \
    test/modbustest.ui \
    test/mqtttest.ui

DISTFILES += \
    python/classification.py

RESOURCES +=
