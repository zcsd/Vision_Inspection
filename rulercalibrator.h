// RulerCalibrator class for ruler calibration automatically
// Author: @ZC
// Date: created on 22 Jun 2018
#ifndef RULERCALIBRATOR_H
#define RULERCALIBRATOR_H

#include <iostream>
#include <string>
#include <iomanip>

#include <QDebug>

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp" // temp using

using namespace cv;
using namespace std;

class RulerCalibrator
{
public:
    RulerCalibrator(cv::Mat& inputFrame, double &pixelDistance);
    ~RulerCalibrator();

private:
    cv::Mat frame, roiFrame, grayFrame, thresholdImage;
    double pixDis = 0.0;
    void preprocessing();
    void thresholding();
    void getContours();
};

#endif // RULERCALIBRATOR_H
