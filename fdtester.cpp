#include "fdtester.h"

FDTester::FDTester(cv::Mat &inputFrame)
{
    originalFrame = inputFrame.clone();
    QElapsedTimer timer;
    timer.start();
    loadRefCtrs();
    qDebug() << timer.elapsed();
    findBestMatch();
    //cv::Mat refImage = imread("../images/FD_Test/vamp_ref.bmp", 1);
    //vector<Point> ref = readRefData();
    //saveRefData(getContour(refImage));
    //compareContours(ref, getContour(originalFrame));
}

FDTester::~FDTester()
{

}

QMap<QString, double> FDTester::getTestDistance()
{
    return testCtrDist;
}

void FDTester::loadRefCtrs()
{
    QString strRefFolder = "../data";
    QDir directory(strRefFolder);
    QStringList refFileList = directory.entryList(QStringList() << "*.txt", QDir::Files);

    refCtrsMap.clear();
    refClassName.clear();

    for (int i = 0; i < refFileList.size(); i++) {
        int pos = refFileList[i].lastIndexOf(QChar('.'));
        refClassName.append(refFileList[i].left(pos));
        refCtrsMap[refFileList[i].left(pos)] = readRefData(strRefFolder+"/"+refFileList[i]);
    }
}

void FDTester::findBestMatch()
{
    vector<Point> testCtr = getContour(originalFrame);
    int classSize = refCtrsMap.size();
    testCtrDist.clear();
    //double minDist = 999999.9999, tempDist = 0;
    for (int i = 0; i < classSize; i++) {
        testCtrDist[refClassName[i]] = compareContours(refCtrsMap[refClassName[i]], testCtr);
    }
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
    // cv::Mat dilateKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, Size(3,3));
    // cv::dilate(thresholdImage, thresholdImage, dilateKernel);

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

double FDTester::compareContours(vector<Point> refContour, vector<Point> testContour)
{
    std::vector<cv::Point2f> refSampleContour, testSampleContour;
    contourSampling(refContour, refSampleContour, 256);
    contourSampling(testContour, testSampleContour, 256);

    FDShapeMatching fdShapeMatching;
    fdShapeMatching.setFDSize(50);

    cv::Mat fd_contour, t;
    //fourierDescriptor(contour_sampled, fd_contour);
    double dist;

    fdShapeMatching.estimateTransformation(refSampleContour, testSampleContour, t, &dist, false);

    //cout << "Distance = " << dist << endl;
    //cout << "Angle = " << t.at<double>(0, 1) * 180 / M_PI <<"\n";
    //cout << "Scale = " << t.at<double>(0, 2)  << "\n";

    return dist;
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

vector<Point> FDTester::readRefData(QString strFilePath)
{
    vector<Point> refCtr;
    refCtr.clear();

    QFile refFile(strFilePath);
    refFile.open(QIODevice::ReadOnly);

    QTextStream in (&refFile);
    QString line;

    while (!in.atEnd()) {
        line = in.readLine();
        QStringList pointSL = line.split("\t");
        refCtr.push_back(Point(pointSL[0].toInt(), pointSL[1].toInt()));
    }

    refFile.close();

    return refCtr;
}

