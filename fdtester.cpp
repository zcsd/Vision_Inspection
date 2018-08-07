#include "fdtester.h"

FDTester::FDTester()
{
    loadRefCtrs();
}

QMap<QString, double> FDTester::getTestDistance(cv::Mat &inputFrame)
{
    originalFrame = inputFrame.clone();
    findMatchResult();
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

void FDTester::findMatchResult()
{
    QElapsedTimer timer;
    timer.start();

    vector<Point> testCtr = getContour(originalFrame);
    int classSize = refCtrsMap.size();
    testCtrDist.clear();

    cv::Mat testSCtr, testSFD;

    contourSampling(testCtr, testSCtr, ctrSize);
    fourierDescriptor(testSCtr, testSFD);
    qDebug() << "Test image preprocessing time: " << timer.elapsed() << "ms";

    QElapsedTimer timer1;
    timer1.start();

    if (!usingThread){
        qDebug() << "!!!NOT using thread!!!";
        // Method NOT using thread
        for (int i = 0; i < classSize; i++) {
            testCtrDist[refClassName[i]] = compareContours(refCtrsMap[refClassName[i]], testSFD);
        }
    } else {
        qDebug() << "!!!Using Thread!!!";
        /*
        // TO OPTIMIZE
        // Method using creating thread Automatically... GOOD
        // but have litte bugs, need to synchronize in fast continuous processing
        for (int i = 0; i < classSize; i++) {
            QtConcurrent::run(compareContours, refClassName[i], refCtrsMap[refClassName[i]], testCtr);
        }
        */

        // Method usingh creating thread manually... BAD
        // Work well for small quantity of classes
        QFuture<double> t0 = QtConcurrent::run(this, &FDTester::compareContours, refCtrsMap[refClassName[0]], testSFD);
        QFuture<double> t1 = QtConcurrent::run(this, &FDTester::compareContours, refCtrsMap[refClassName[1]], testSFD);
        QFuture<double> t2 = QtConcurrent::run(this, &FDTester::compareContours, refCtrsMap[refClassName[2]], testSFD);
        QFuture<double> t3 = QtConcurrent::run(this, &FDTester::compareContours, refCtrsMap[refClassName[3]], testSFD);
        QFuture<double> t4 = QtConcurrent::run(this, &FDTester::compareContours, refCtrsMap[refClassName[4]], testSFD);

        t0.waitForFinished();
        t1.waitForFinished();
        t2.waitForFinished();
        t3.waitForFinished();
        t4.waitForFinished();

        testCtrDist[refClassName[0]] = t0.result();
        testCtrDist[refClassName[1]] = t1.result();
        testCtrDist[refClassName[2]] = t2.result();
        testCtrDist[refClassName[3]] = t3.result();
        testCtrDist[refClassName[4]] = t4.result();
    }
     qDebug() << "Total matching time: " << timer1.elapsed() << "ms";
}

vector<Point> FDTester::getContour(Mat image)
{
    cv::Mat smallImage, grayImage, thresholdImage;
    cv::Scalar WHITE = Scalar(255, 255, 255);

    cv::resize(image, smallImage, Size(), 0.25, 0.25, INTER_LINEAR);
    cv::cvtColor(smallImage, grayImage, COLOR_BGR2GRAY);
    cv::GaussianBlur(grayImage, grayImage, Size(3, 3), 0);
    cv::threshold(grayImage, thresholdImage, 135, 255, THRESH_BINARY_INV);
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

    //cv::imwrite("tt.jpg", thresholdImage);

    return contours[indexMaxContour];
}

double FDTester::compareContours(vector<Point> refCtr, Mat srcFD)
{
    QElapsedTimer timer;
    timer.start();

    cv::Mat refSCtr, refFD;
    contourSampling(refCtr, refSCtr, ctrSize);
    fourierDescriptor(refSCtr, refFD);

    FDShapeMatching fdShapeMatching(ctrSize, fdSize);

    cv::Mat t;
    double dist;

    fdShapeMatching.estimateTransformation(srcFD, refFD, &t, &dist);

    //cout << "Distance = " << dist << endl;
    //cout << "Angle = " << t.at<double>(0, 1) * 180 / M_PI <<"\n";
    //cout << "Scale = " << t.at<double>(0, 2)  << "\n";
    qDebug() << "One matching time(" << QThread::currentThreadId() << "):" << timer.elapsed() << "ms";
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

FDTester::~FDTester()
{

}
