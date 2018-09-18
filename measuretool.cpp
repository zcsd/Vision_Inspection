#include "measuretool.h"

MeasureTool::MeasureTool(QObject *parent) : QObject(parent)
{

}

void MeasureTool::receiveFrame(Mat frame)
{
    frameCopy = frame.clone();
    //cv::imwrite("../images/gsd.jpg", frameCopy);
}
