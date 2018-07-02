// FDTester class: Fourier Descriptor Based Shaped Matching
// Author: @ZC
// Date: created on 02 Jul 2018
#ifndef FDTESTER_H
#define FDTESTER_H

#include <iostream>

#include <QDebug>
#include <QElapsedTimer>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/ximgproc.hpp>

using namespace cv;
using namespace std;

class FDTester
{
public:
    FDTester(cv::Mat &inputFrame);
    ~FDTester();

private:
    cv::Mat originalFrame;
    void process();

};

#endif // FDTESTER_H
