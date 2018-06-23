// RulerCalibrator class for ruler calibration automatically
// Author: @zichun
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
    RulerCalibrator(cv::Mat& inputFrame, const double& realDistance, double& autoPPMM);

private:
    cv::Mat frame, roiFrame, grayFrame, thresholdImage;
    double distanceInMM;
    double PPMM = 0.0;
    void preprocessing();
    void thresholding();
    void getContours();
    void drawContours();
};

#endif // RULERCALIBRATOR_H
