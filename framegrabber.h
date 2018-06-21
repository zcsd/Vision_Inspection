// FrameGrabber class for Basler USB3.0 camera
// Author: @zichun
// Date: created on 18 Jun 2018
#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include <iostream>
#include <unistd.h> // for usleep()

#include <QObject>
#include <QDebug>
#include <QString>
#include <QImage>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
// Namespace for using pylon objects.
using namespace Pylon;
// Settings for using Basler USB cameras.
#include <pylon/usb/BaslerUsbInstantCamera.h>
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
typedef Camera_t::GrabResultPtr_t GrabResultPtr_t;
using namespace Basler_UsbCameraParams;

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core/core.hpp"

using namespace cv;
using namespace std;

class FrameGrabber : public QObject
{
    Q_OBJECT

public:
    explicit FrameGrabber(QObject *parent = nullptr);
    ~FrameGrabber();
    bool cameraConnected;
    bool startGrabbing;
    QString scanDevices();

private:
    Camera_t *pylonCamera;
    char grabMode;
    cv::Mat cvFrame;
    cv::Mat readFrame();
    void initCamera();
    void configureCamera();

signals:
    void sendCaptureFrame(cv::Mat cvFrameToSend);
    void sendFrame(cv::Mat cvFrameToSend);

public slots:
    void receiveConnectCamera();
    void receiveDisconnectCamera();
    void receiveStartCaptureMode();
    void receiveStartStreamMode();
    void receiveStopGrabbing();
    void receiveSendFrame();
};

#endif // FRAMEGRABBER_H
