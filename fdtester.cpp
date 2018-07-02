#include "fdtester.h"

FDTester::FDTester(cv::Mat &inputFrame)
{
    originalFrame = inputFrame.clone();
    process();
}

FDTester::~FDTester()
{

}

void FDTester::process()
{
    QElapsedTimer timer;
    timer.start();

    cv::Mat smallImage, grayImage, thresholdImage;
    cv::Scalar WHITE = Scalar(255, 255, 255);

    cv::resize(originalFrame, smallImage, Size(), 0.5, 0.5, INTER_LINEAR);
    cv::cvtColor(smallImage, grayImage, COLOR_BGR2GRAY);
    // cv::blur(gray_img, gray_img, cv::Size(3,3));
    cv::threshold(grayImage, thresholdImage, 80, 255, THRESH_BINARY_INV);
    // kernel shape: MORPH_RECT   MORPH_CROSS  MORPH_ELLIPSE
    cv::Mat dilate_kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, Size(3,3));
    cv::dilate(thresholdImage, thresholdImage, dilate_kernel);

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(thresholdImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));

    std::vector<cv::Rect> bound_rect(contours.size());
    cv::Mat contoursImage = Mat::zeros(thresholdImage.size(), CV_8UC3);

    for (size_t cC = 0; cC < contours.size(); cC++) {
       cv::drawContours(contoursImage, contours, (int)cC, WHITE, 1, 8, hierarchy, 0, Point());
       /*
       bound_rect[cC] = cv::boundingRect(contours[cC]);
       // @ZC TODO not sure if it is necessary
       for (size_t cP = 0; cP < contours[cC].size(); cP++) {
           contours[cC][cP].x = contours[cC][cP].x - bound_rect[cC].x;
           contours[cC][cP].y = contours[cC][cP].y - bound_rect[cC].y;
       }
       */
    }

    cout << contours[0].size() << endl;
    cv::ximgproc::ContourFitting fd;
    std::vector<cv::Point2f> contour_sampled;
    cv::ximgproc::contourSampling(contours[0], contour_sampled, 1670);
    fd.setFDSize(50);
    cv::Mat fd_contour, t;
    cv::ximgproc::fourierDescriptor(contour_sampled, fd_contour);
    double dist;

    cout << contours[0].size() << endl;
    fd.estimateTransformation(fd_contour, fd_contour, t, &dist, true);
    cout << contours[0].size() << endl;
    cout << dist << endl;
    cout << "Angle = " << t.at<double>(0, 1) * 180 / M_PI <<"\n";
    cout << "Scale = " << t.at<double>(0, 2)  << "\n";

    //cout << fd_contour.at<double>(0, 0) << "  "  << endl;
    //const char* filename = "output.txt";

    //writeMatToFile(fd_contour,filename);
    qDebug() << timer.elapsed() << "ms";
    cv::imshow("1", contoursImage);
    cv::waitKey(0);


}
