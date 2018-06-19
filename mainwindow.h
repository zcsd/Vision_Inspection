// Author: @zichun-SAT
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

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"

#include "framegrabber.h"

using namespace std;
using namespace cv;

class FrameGrabber;

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

private:
    Ui::MainWindow *ui;
    char grabMode;
    cv::Mat cvRawFrame;
    cv::Mat cvRawFrameCopy;
    cv::Mat cvResizedFrame;
    QImage qDisplayedFrame;
    QTimer *streamTrigger;
    void initialSetup();
    void displayFrame(cv::Mat cvDisplayFrame);
};

#endif // MAINWINDOW_H
