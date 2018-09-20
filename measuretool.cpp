#include "measuretool.h"

MeasureTool::MeasureTool(QObject *parent) : QObject(parent)
{
    bgImage = imread("../images/BG_green.jpg", 1);
    ROI = Rect(10, 150, 2428, 1600); //2448x2048, 10, 150, 2428, 1600
}

void MeasureTool::receiveFrame(Mat frame)
{
    frameCopy = frame.clone();
    diffSegmentation();
}

void MeasureTool::receiveCalibrationPara(double pPmm, int test)
{
    pixelPERmm = pPmm;
    //qDebug() << QString::number(pixelPERmm) << QString::number(test);
}

void MeasureTool::diffSegmentation()
{
    Mat roiBG = bgImage(ROI).clone();
    Mat roiObj = frameCopy(ROI).clone();

    cv::cvtColor(roiBG, roiBG, COLOR_BGR2HSV);
    cv::cvtColor(roiObj, roiObj, COLOR_BGR2HSV);

    cv::Mat channels[3];
    split(roiObj,channels);
    //channels[0] = Mat::zeros(roiObj.rows, roiObj.cols, CV_8UC1);
    channels[1] = Mat::zeros(roiObj.rows, roiObj.cols, CV_8UC1);
    //channels[2] = Mat::zeros(roiObj.rows, roiObj.cols, CV_8UC1);
    merge(channels, 3, roiObj);
    //cv::cvtColor(roiObj, roiObj, COLOR_HSV2BGR);

    cv::Mat channels1[3];
    split(roiBG,channels1);
    //channels1[0] = Mat::zeros(roiBG.rows, roiBG.cols, CV_8UC1);
    channels1[1] = Mat::zeros(roiBG.rows, roiBG.cols, CV_8UC1);
    //channels1[2] = Mat::zeros(roiBG.rows, roiBG.cols, CV_8UC1);
    merge(channels1, 3, roiBG);
    //cv::cvtColor(roiBG, roiBG, COLOR_HSV2BGR);

    cv::Mat resImage, thresholdImage;
    cv::absdiff(roiObj, roiBG, resImage);
    cv::cvtColor(resImage, resImage, COLOR_HSV2BGR);
    cv::cvtColor(resImage,resImage, COLOR_BGR2GRAY);
    cv::threshold(resImage, thresholdImage, 32, 255, THRESH_BINARY);

    cv::Mat showImg;
    cv::resize(thresholdImage, showImg, cv::Size(), 0.5, 0.5);
    cv::namedWindow("test", 1);
    while (true)
    {
        cv::imshow("test", showImg);
        if ( (cv::waitKey(1) & 0xFF) == 27 ) break;
    }
    cv::destroyWindow("test");
}
