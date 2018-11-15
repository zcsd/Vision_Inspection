#include "measuretool.h"

MeasureTool::MeasureTool(QObject *parent) : QObject(parent)
{
    bgImage = imread("../images/BG_green.jpg", 1);
    ROI = Rect(4, 4, 2440, 2040); //2448x2048
}

void MeasureTool::receiveFrame(Mat frame)
{
    frameCopy = frame.clone();
    //cannySegmentation();

    diffSegmentation();
    getContours();

    cv::Mat frameToSend = frameCopy.clone();
    roiShow.copyTo(frameToSend(ROI));
    //cv::imwrite("../images/tes.jpg", frameToSend);

    emit sendFrameToShow(frameToSend);
    emit sendMeasurement(realDistance);
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
    roiShow = frameCopy(ROI).clone();
    //cv::imwrite("../images/tes.jpg", roiObj);
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

    cv::Mat resImage;
    cv::absdiff(channels[0], channels1[0], resImage);
    //cv::cvtColor(resImage, resImage, COLOR_HSV2BGR);
    //cv::cvtColor(resImage,resImage, COLOR_BGR2GRAY);
    cv::threshold(resImage, thresholdImage, 30, 255, THRESH_BINARY);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, Size(3,3));
    cv::erode(thresholdImage, thresholdImage, kernel);
    cv::dilate(thresholdImage, thresholdImage, kernel);
    //cv::dilate(thresholdImage, thresholdImage, kernel);

    cv::Mat showImg;
    cv::resize(thresholdImage, showImg, cv::Size(), 0.5, 0.5);
    cv::namedWindow("test", 1);
    while (true)
    {
        cv::imshow("test", showImg);
        if ( (cv::waitKey(1) & 0xFF) == 'q' ) break;
    }
    cv::destroyWindow("test");
}

void MeasureTool::getContours()
{
    //***************TEST********************//
/*    cv::Mat test = imread("../images/test.png", 0);
    threshold(test, thresholdImage, 20, 255, THRESH_BINARY);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, Size(3,3));
    cv::erode(thresholdImage, thresholdImage, kernel);
    cv::dilate(thresholdImage, thresholdImage, kernel);*/
    //**************TEST END*****************//

    vector<vector<Point>> contours;
    vector<Point> maxCtr;
    vector<Vec4i> hierarchy;
    cv::RotatedRect rotatedRect;

    cv::findContours(thresholdImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
    maxCtr = getMaxContour(contours);

    rotatedRect = cv::minAreaRect(Mat(maxCtr));
    Point2f rectPoints[4];
    rotatedRect.points(rectPoints);
    for (int j = 0; j < 4; j++)
       cv::line(roiShow, rectPoints[j], rectPoints[(j+1)%4], Scalar(255, 0, 0), 2, 8);

    double maxLengh = 0.0;

    if (rotatedRect.size.height >= rotatedRect.size.width)
    {
        maxLengh = double(rotatedRect.size.height);
    }
    else
    {
        maxLengh = double(rotatedRect.size.width);
    }

    realDistance = maxLengh / pixelPERmm;

    // Create an output string stream
    std::ostringstream streamObj3;
    // Set Fixed -Point Notation
    streamObj3 << std::fixed;
    // Set precision to 1 digits
    streamObj3 << std::setprecision(2);
    //Add double to stream
    streamObj3 << realDistance;
    // Get string from output string stream
    std::string strObj3 = streamObj3.str();
    string printDistance = strObj3 + "mm";
    cv::putText(roiShow, printDistance, rotatedRect.center, 2, 4.0, Scalar(0, 255, 0), 2, 8);
/*
    cv::Mat showImg;
    cv::resize(roiShow, showImg, cv::Size(), 0.5, 0.5);
    cv::namedWindow("test", 1);
    while (true)
    {
        cv::imshow("test", showImg);
        if ( (cv::waitKey(1) & 0xFF) == 'q' ) break;
    }
    cv::destroyWindow("test");*/
}

vector<Point> MeasureTool::getMaxContour(vector<vector<Point> > allContours)
{
    std::vector<cv::Point> maxContour;
    double maxArea = -999999.99;
    int maxIndex = -1;
    for (size_t i = 0; i < allContours.size(); i++)
    {
        if (cv::contourArea(allContours[i]) >= maxArea)
        {
            maxArea = cv::contourArea(allContours[i]);
            maxContour = allContours[i];
            maxIndex = int(i);
        }
    }
    //cv::drawContours(roiShow, allContours, maxIndex, Scalar(0, 0, 255), 2, 8, vector<Vec4i>(), 0, Point(0,0));

    return maxContour;
}

void MeasureTool::cannySegmentation()
{
    Mat roiObj = frameCopy(ROI).clone();
    roiShow = frameCopy(ROI).clone();

    Mat grayImage, cannyImage;
    cv::cvtColor(roiObj, roiObj, COLOR_BGR2HSV);

    cv::Mat channels[3];
    split(roiObj,channels);
    //channels[0] = Mat::zeros(roiBG.rows, roiBG.cols, CV_8UC1);
    channels[1] = Mat::zeros(roiObj.rows, roiObj.cols, CV_8UC1);
    //channels[2] = Mat::zeros(roiBG.rows, roiBG.cols, CV_8UC1);
    merge(channels, 3, roiObj);
    cv::cvtColor(roiObj, roiObj, COLOR_HSV2BGR);
    cv::cvtColor(roiObj, roiObj, COLOR_BGR2GRAY);

    cv::Canny(roiObj, cannyImage, 20, 60, 3);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, Size(3,3));
    cv::dilate(cannyImage, cannyImage, kernel);
    cv::erode(cannyImage, cannyImage, kernel);
/*
    cv::Mat showImg;
    cv::resize(cannyImage, showImg, cv::Size(), 0.5, 0.5);
    cv::namedWindow("test", 1);
    while (true)
    {
        cv::imshow("test", showImg);
        if ( (cv::waitKey(1) & 0xFF) == 'q' ) break;
    }
    cv::destroyWindow("test");*/
}
