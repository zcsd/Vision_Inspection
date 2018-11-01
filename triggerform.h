// TriggerForm class: detect coming parts to trigger basler camera to capture
// Author: @ZC
// Date: created on 02 Oct 2018
#ifndef TRIGGERFORM_H
#define TRIGGERFORM_H

#include <QWidget>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QThread>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
using namespace cv;

#include <stdio.h>
using namespace std;

namespace Ui {
class TriggerForm;
}

class TriggerForm : public QWidget
{
    Q_OBJECT

public:
    explicit TriggerForm(QWidget *parent = nullptr);
    ~TriggerForm();

signals:
    void sendTrigger();

private slots:
    void on_pushButtonStart_clicked();
    void on_pushButtonStop_clicked();
    void on_pushButtonReset_clicked();
    void on_pushButtonWork_clicked();
    void receiveUpdateFrame();

private:
    Ui::TriggerForm *ui;
    QTimer *camTrigger;
    QThread *workThread;
    cv::VideoCapture capture;
    QPixmap whitePixmap;
    cv::Mat frame;
    cv::Mat bgImg, bgImgROI;
    cv::Rect ROI;
    int partsCounter = 0;
    int frameCounter = 0;
    bool startCount = false;
    bool startWork = false;
    void initUSBCam();
    void releaseUSBCam();
    cv::Mat processFrame(cv::Mat img);
};

#endif // TRIGGERFORM_H
