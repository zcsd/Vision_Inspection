// CalibratorForm class: Do all calibration steps
// Author: @ZC
// Date: created on 05 Sep 2018
#ifndef CALIBRATORFORM_H
#define CALIBRATORFORM_H

#include <QWidget>
#include <QDebug>
#include <QMessageBox>

#include <unistd.h> // for usleep()
#include <math.h>
#include <iostream>
using namespace std;

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
using namespace cv;

namespace Ui {
class CalibratorForm;
}

class CalibratorForm : public QWidget
{
    Q_OBJECT

public:
    explicit CalibratorForm(QWidget *parent = 0);
    ~CalibratorForm();

signals:
    void sendFrameRequest();
    void sendFrameToShow(cv::Mat frame);

public slots:
    void receiveFrame(cv::Mat frame);

private slots:
    void on_pushButtonBGStart_clicked();
    void on_pushButtonRulerStart_clicked();
    void on_pushButtonCalculate_clicked();
    void receiveCloseForm();
    void receiveSetButtonVisible(QString input);

private:
    Ui::CalibratorForm *ui;
    cv::Mat frameCopy, roiBGFrame, roiRLFrame, roiRLFrame2Show;
    cv::Mat thresholdImg, contourImg;
    cv::Scalar meanBGR, meanHSV, meanGS;
    cv::Rect ROI;
    double pixelDistance, pixelPERmm;
    void initialSetup();
    void extractColorMean();
    void autoCalibrateRuler();
    void manualCalibrateRuler();
    void hsvThreshold();
    void diffThreshold();
    void grayscaleThreshold();
    void getContour();
};

#endif // CALIBRATORFORM_H
