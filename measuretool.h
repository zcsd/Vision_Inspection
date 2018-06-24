#ifndef MEASURETOOL_H
#define MEASURETOOL_H

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

class MeasureTool
{

public:
    MeasureTool(cv::Mat& inputFrame, const double ppmm);

private:
    cv::Mat frame, roiFrame, grayFrame, thresholdImage;
    double newPPMM;
    void preprocessing();
    void thresholding();
    void getContours();


};

#endif // MEASURETOOL_H
