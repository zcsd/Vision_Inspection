#include "fdtester.h"

FDTester::FDTester(cv::Mat &inputFrame)
{
    originalFrame = inputFrame.clone();
    //cv::Mat refImage = imread("../images/FD_Test/vamp_ref.bmp", 1);
    vector<Point> ref = readRefData();
    //saveRefData(getContour(refImage));
    compareContours(ref, getContour(originalFrame));
}

FDTester::~FDTester()
{

}

vector<Point> FDTester::getContour(Mat image)
{
    cv::Mat smallImage, grayImage, thresholdImage;
    cv::Scalar WHITE = Scalar(255, 255, 255);

    cv::resize(image, smallImage, Size(), 0.5, 0.5, INTER_LINEAR);
    cv::cvtColor(smallImage, grayImage, COLOR_BGR2GRAY);
    cv::GaussianBlur(grayImage, grayImage, Size(3, 3), 0);
    cv::threshold(grayImage, thresholdImage, 135, 210, THRESH_BINARY_INV);
    // kernel shape: MORPH_RECT   MORPH_CROSS  MORPH_ELLIPSE
    //cv::Mat dilateKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, Size(3,3));
    //cv::dilate(thresholdImage, thresholdImage, dilateKernel);

    std::vector<vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(thresholdImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));

    cv::Mat contoursImage = Mat::zeros(thresholdImage.size(), CV_8UC3);
    int indexMaxContour = 0;
    double maxArea = -999, tempArea;

    for (size_t cC = 0; cC < contours.size(); cC++) {
       cv::drawContours(contoursImage, contours, (int)cC, WHITE, 1, 8, hierarchy, 0, Point());
       tempArea = contourArea(contours[cC]);
       if (tempArea >= maxArea) {
           indexMaxContour = cC;
           maxArea = tempArea;
       }
    }

    return contours[indexMaxContour];
}

void FDTester::compareContours(vector<Point> refContour, vector<Point> testContour)
{
    std::vector<cv::Point2f> refSampleContour, testSampleContour;
    cv::ximgproc::contourSampling(refContour, refSampleContour, 256);
    cv::ximgproc::contourSampling(testContour, testSampleContour, 256);

    cv::ximgproc::ContourFitting fd;
    fd.setFDSize(50);

    cv::Mat fd_contour, t;
    //cv::ximgproc::fourierDescriptor(contour_sampled, fd_contour);
    double dist;

    fd.estimateTransformation(refSampleContour, testSampleContour, t, &dist, false);

    cout << "Distance = " << dist << endl;
    cout << "Angle = " << t.at<double>(0, 1) * 180 / M_PI <<"\n";
    cout << "Scale = " << t.at<double>(0, 2)  << "\n";
}

void FDTester::saveRefData(vector<Point> refContour)
{
    QFile refFile("../data/temp.txt");
    refFile.open(QIODevice::WriteOnly|QIODevice::Truncate);

    QTextStream out (&refFile);

    int sizeContour = int(refContour.size());
    for (int i = 0; i < sizeContour; i++) {
        out << QString::number(refContour[i].x) + "\t" + QString::number(refContour[i].y) + "\n";
    }
    refFile.close();
}

vector<Point> FDTester::readRefData()
{
    vector<Point> refC;
    refC.clear();

    QFile refFile("../data/p.txt");
    refFile.open(QIODevice::ReadOnly);

    QTextStream in (&refFile);
    QString line;

    while (!in.atEnd()) {
        line = in.readLine();
        QStringList pointSL = line.split("\t");
        refC.push_back(Point(pointSL[0].toInt(), pointSL[1].toInt()));
    }

    refFile.close();

    return refC;
}
