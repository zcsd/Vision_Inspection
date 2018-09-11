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
#include <QThread>
#include <QtConcurrent>
#include <QObject>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include "fourierdescriptor.h"

using namespace cv;
using namespace std;

class FDTester: public QObject
{
    Q_OBJECT

public:
    FDTester();
    ~FDTester();
    QMap<QString, double> getTestDistance(cv::Mat &inputFrame);

private:
    FDShapeMatching *fdShapeMatching;
    bool usingThread = false;
    int ctrSize = 128, fdSize = 12;
    cv::Mat originalFrame;
    QStringList refClassName;
    QMap<QString, vector<Point>> refCtrsMap;
    QMap<QString, double> testCtrDist;
    void loadRefCtrs();
    void saveRefData(vector<Point> refContour);
    void findMatchResult();
    vector<Point> getContour(cv::Mat image);
    vector<Point> readRefData(QString strFilePath);
    double compareContours(vector<Point> refCtr, Mat srcFD);

signals:
    //void sendResult(QString name, double distance);

private slots:
    //void receiveResult(QString name, double distance);

};

#endif // FDTESTER_H
