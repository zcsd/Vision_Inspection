// MainWindow class
// Author: @zichun
// Date: created on 18 Jun 2018
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>

#include <QMainWindow>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QString>
#include <QImage>
#include <QTimer>
#include <QTime>
#include <QString>
#include <QFileDialog>

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"

#include "framegrabber.h"
#include "cameracalibrator.h"

using namespace std;
using namespace cv;

class FrameGrabber;
class CameraCalibrator;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    FrameGrabber *frameGrabber;
    CameraCalibrator *cameraCalibrator;

signals:
    void sendConnect();
    void sendDisconnect();
    void sendCaptureMode();
    void sendStreamMode();
    void sendStopGrabbing();

private slots:
    void on_pushButtonConnect_clicked();
    void on_pushButtonDisconnect_clicked();
    void on_pushButtonCapture_clicked();
    void on_pushButtonStream_clicked();
    void on_pushButtonStop_clicked();
    void on_pushButtonScanDevices_clicked();
    void receiveRawFrame(cv::Mat cvRawFrame);
    void on_pushButtonSaveCapture_clicked();
    void on_pushButtonCalibrate_clicked();
    void on_actionChangeSavePath_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomToFit_triggered();
    void on_actionZoomOut_triggered();
    void on_actionZoomToRaw_triggered();
    void on_actionMCalibrate_triggered();

    void on_pushButtonLine1_clicked();

    void on_actionOpenImage_triggered();

public slots:
    void receiveShowMousePosition(QPoint& pos);

private:
    Ui::MainWindow *ui;
    QString defaultSavePath = "../images";
    char grabMode;
    bool manualCalibration = false;
    double scaleFactor = 1.0;
    cv::Mat cvRawFrameCopy;
    cv::Mat cvRGBFrame;
    QImage qDisplayedFrame;
    QTimer *streamTrigger;
    void initialSetup();
    void displayFrame();
};

#endif // MAINWINDOW_H
