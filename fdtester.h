// FDTester class: Fourier Descriptor Based Shaped Matching
// Author: @ZC
// Date: created on 02 Jul 2018
#ifndef FDTESTER_H
#define FDTESTER_H

#include <iostream>

#include <QDebug>
#include <QFile>
#include <QElapsedTimer>
#include <QDir>

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

    QMap<QString, double> getTestDistance();

private:
    cv::Mat originalFrame;
    void loadRefCtrs();
    void findBestMatch();
    vector<Point> getContour(cv::Mat image);
    double compareContours(vector<Point> refContour, vector<Point> testContour);
    void saveRefData(vector<Point> refContour);
    vector<Point> readRefData(QString strFilePath);
    QMap<QString, vector<Point>> refCtrsMap;
    QMap<QString, double> testCtrDist;
    QStringList refClassName;

};

#endif // FDTESTER_H
