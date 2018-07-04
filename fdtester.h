// FDTester class: Fourier Descriptor Based Shaped Matching
// Author: @ZC
// Date: created on 02 Jul 2018
#ifndef FDTESTER_H
#define FDTESTER_H

#include <iostream>

#include <QDebug>
#include <QFile>
#include <QElapsedTimer>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include "fourierdescriptor.h"

using namespace cv;
using namespace std;

class FDTester
{
public:
    FDTester(cv::Mat &inputFrame);
    ~FDTester();

private:
    cv::Mat originalFrame;
    vector<Point> getContour(cv::Mat image);
    void compareContours(vector<Point> refContour, vector<Point> testContour);
    void saveRefData(vector<Point> refContour);
    vector<Point> readRefData();

};

#endif // FDTESTER_H
