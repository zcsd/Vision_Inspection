#ifndef MEASURETOOL_H
#define MEASURETOOL_H

#include <QObject>

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
using namespace cv;

class MeasureTool : public QObject
{
    Q_OBJECT
public:
    explicit MeasureTool(QObject *parent = nullptr);

signals:

public slots:
    void receiveFrame(cv::Mat frame);

private:
    cv::Mat frameCopy;
};

#endif // MEASURETOOL_H
