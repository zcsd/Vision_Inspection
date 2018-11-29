// MeasureTool class: measure part size
// Author: @ZC
// Date: created on 20 Sep 2018
#ifndef MEASURETOOL_H
#define MEASURETOOL_H

#include <QObject>
#include <QDebug>

#include <stdio.h>
using namespace std;

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
using namespace cv;

class MeasureTool : public QObject
{
    Q_OBJECT
public:
    explicit MeasureTool(QObject *parent = nullptr);

signals:
    void sendMeasurement(double length, bool color);
    void sendFrameToShow(cv::Mat image);

public slots:
    void receiveFrame(cv::Mat frame);
    void receiveCalibrationPara(double pPmm, int test);

private:
    cv::Mat frameCopy, bgImage, roiShow, thresholdImage;
    cv::Rect ROI;
    double pixelPERmm, realDistance;
    bool isColorOK = true, isContourExist = true;
    void diffSegmentation();
    void getContours();
    vector<Point> getMaxContour(vector<vector<Point>> allContours);
    void cannySegmentation();
    bool checkColor(cv::Mat image);
};

#endif // MEASURETOOL_H
