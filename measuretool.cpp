#include "measuretool.h"

MeasureTool::MeasureTool(cv::Mat& inputFrame, const double ppmm)
{
    frame = inputFrame.clone();
    newPPMM = ppmm;
    preprocessing();
    thresholding();
    getContours();
    inputFrame = frame;
}

void MeasureTool::preprocessing()
{
    cv::Rect ROI = Rect(250, 280, 1600, 930);
    roiFrame = frame(ROI);
    cv::cvtColor(roiFrame, grayFrame, COLOR_BGR2GRAY);
    cv::GaussianBlur(grayFrame, grayFrame, Size(3, 3), 0);
}

void MeasureTool::thresholding()
{
    //cv::Canny(grayFrame, cannyImage, 80, 200, 3); // edge pnly
    cv::threshold(grayFrame, thresholdImage, 135, 210, THRESH_BINARY_INV);
    //cv::dilate(thresholdImage, thresholdImage, Mat());
    //cv::erode(thresholdImage, thresholdImage, Mat());
}

void MeasureTool::getContours()
{
    std::vector<vector<Point>> contours;
    std::vector<Vec4i> hierarchy;
    cv::findContours(thresholdImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
    std::vector<cv::Rect> boundRect( contours.size() );
    std::vector<vector<Point> > contours_poly( contours.size() );
    std::vector<Point2f> center( contours.size() );
    std::vector<float> radius( contours.size() );
    for (size_t i = 0; i < contours.size(); i++)
    {
        boundRect[i] = boundingRect(contours[i]);
        if (boundRect[i].area() > 100) {
            cv::drawContours(roiFrame, contours, i, Scalar(255, 0, 0), 2, 8, vector<Vec4i>(), 0, Point());
            cv::rectangle(roiFrame, boundRect[i], Scalar(0, 0, 255), 2, 8);
            //qDebug() << boundRect[i].width << boundRect[i].height;
            double realDistance = boundRect[i].width / newPPMM;
            //qDebug() << realDistance;
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
            cv::putText(roiFrame, printDistance, Point(boundRect[i].x + (boundRect[i].width/2), boundRect[i].y), 1, 1.5, Scalar(0, 255, 0), 2, 8);
        }

    }

    //cv::imwrite("../images/aa.bmp", thresholdImage);

    //cv::namedWindow("test", 1);
    //cv::imshow("test", thresholdImage);
    //cv::waitKey(0);
}
