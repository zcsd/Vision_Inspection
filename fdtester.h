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
    FDTester(cv::Mat &inputFrame);
    ~FDTester();
    QMap<QString, double> getTestDistance();

private:
    cv::Mat originalFrame;
    QStringList refClassName;
    QMap<QString, vector<Point>> refCtrsMap;
    QMap<QString, double> testCtrDist;
    void loadRefCtrs();
    void saveRefData(vector<Point> refContour);
    void findMatchResult();
    vector<Point> getContour(cv::Mat image);
    vector<Point> readRefData(QString strFilePath);
    static void compareContours(QString className, vector<Point> refContour, vector<Point> testContour);

signals:
    static void sendResult(QString name, double distance);

private slots:
    void receiveResult(QString name, double distance);

};

#endif // FDTESTER_H
