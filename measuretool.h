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

public slots:
    void receiveFrame(cv::Mat frame);
    void receiveCalibrationPara(double pPmm, int test);

private:
    cv::Mat frameCopy, bgImage;
    cv::Rect ROI;
    double pixelPERmm;
    void diffSegmentation();
};

#endif // MEASURETOOL_H
