#include "calibratorform.h"
#include "ui_calibratorform.h"

CalibratorForm::CalibratorForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalibratorForm)
{
    ui->setupUi(this);
    initialSetup();
}

CalibratorForm::~CalibratorForm()
{
    delete ui;
}

void CalibratorForm::initialSetup()
{
    QDoubleValidator *validator = new QDoubleValidator(0.00, 9999.99, 2);
    ui->lineEditmmD->setValidator(validator);

    connect(ui->buttonBoxEnd, SIGNAL(rejected()), this, SLOT(receiveCloseForm()));
    // set button visible or not
    receiveSetButtonVisible(ui->comboBoxCaliColor->currentText());
    receiveSetButtonVisible(ui->comboBoxCaliMethod->currentText());
    connect(ui->comboBoxCaliColor, SIGNAL(activated(QString)), this, SLOT(receiveSetButtonVisible(QString)));
    connect(ui->comboBoxCaliMethod, SIGNAL(activated(QString)), this, SLOT(receiveSetButtonVisible(QString)));
    // @ZC, temp using!!! for step 1 and 2
    ROI = Rect(10, 150, 2428, 1600); // original 2448x2048, now 2428x1600
}

void CalibratorForm::receiveFrame(cv::Mat frame)
{
    frameCopy = frame.clone();
}

void CalibratorForm::on_pushButtonBGStart_clicked()
{
    emit sendFrameRequest();
    usleep(2000);
    extractColorMean();
}

void CalibratorForm::extractColorMean()
{
    roiBGFrame = frameCopy(ROI).clone();

    cv::Mat roiFrameHSV, roiFrameGS;
    cv::cvtColor(roiBGFrame, roiFrameHSV, COLOR_BGR2HSV_FULL);
    cv::cvtColor(roiBGFrame, roiFrameGS, COLOR_BGR2GRAY);
    meanGS = cv::mean(roiFrameGS);
    meanBGR = cv::mean(roiBGFrame); // B G R order
    meanHSV = cv::mean(roiFrameHSV); // H S V order
    QString grayVauleStr = QString::number(int(meanGS[0]));
    QString bgrValueStr = QString::number(int(meanBGR[0])) + "," + QString::number(int(meanBGR[1]))
                          + "," + QString::number(int(meanBGR[2]));
    QString hsvValueStr = QString::number(int(meanHSV[0])) + "," + QString::number(int(meanHSV[1]))
                          + "," + QString::number(int(meanHSV[2]));
    QString rgbValueSS =  "background-color: rgb(" + QString::number(int(meanBGR[2])) + ", "
                          + QString::number(int(meanBGR[1])) + ", " + QString::number(int(meanBGR[0])) + ");";
    ui->labelShowRGB->setText(bgrValueStr);
    ui->labelShowHSV->setText(hsvValueStr);
    ui->labelShowGS->setText(grayVauleStr);
    ui->labelDisplayColor->setStyleSheet(rgbValueSS);
}

void CalibratorForm::on_pushButtonRulerStart_clicked()
{
    emit sendFrameRequest(); // require 2nd frame for step 2
    usleep(2000);
    roiRLFrame = frameCopy(ROI).clone();

    if (ui->comboBoxCaliMethod->currentText() == "Auto")
    {
        autoCalibrateRuler();
    }
    else if (ui->comboBoxCaliMethod->currentText() == "Manual")
    {
        manualCalibrateRuler();
    }
}

void CalibratorForm::autoCalibrateRuler()
{

    if (ui->comboBoxCaliColor->currentText() == "HSV")
    {
        hsvThreshold();
    }
    else if (ui->comboBoxCaliColor->currentText() == "GrayScale")
    {
        grayscaleThreshold();
    }
    else if (ui->comboBoxCaliColor->currentText() == "Diff")
    {
        diffThreshold();
    }

    getContour();

    ui->labelShowPixelD->setText(QString::number(pixelDistance, 'f', 1));
    cv::Mat frameToSend = frameCopy.clone();
    roiRLFrame2Show.copyTo(frameToSend(ROI));

    emit sendFrameToShow(frameToSend);
}

void CalibratorForm::manualCalibrateRuler()
{

}

void CalibratorForm::hsvThreshold()
{
    cv::Mat roiRLHSV, thresholdHSV, roiRLFrameB;
    cv::GaussianBlur(roiRLFrame, roiRLFrameB, Size(3, 3), 0);
    cv::cvtColor(roiRLFrameB, roiRLHSV, COLOR_BGR2HSV_FULL);
    int h = meanHSV[0], s = meanHSV[1], v = meanHSV[2];
    cv::inRange(roiRLHSV, Scalar(h-40, s-100, v-100), Scalar(h+40, s+100, v+100), thresholdHSV);
    cv::bitwise_not(thresholdHSV, thresholdImg);
    // kernel shape: MORPH_RECT   MORPH_CROSS  MORPH_ELLIPSE
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, Size(3,3));
    cv::erode(thresholdImg, thresholdImg, kernel);
    cv::dilate(thresholdImg, thresholdImg, kernel);
    //cv::imwrite("../images/hsvT.jpg", thresholdImg);
}

void CalibratorForm::grayscaleThreshold()
{
    cv::Mat roiRLGray;
    cv::cvtColor(roiRLFrame, roiRLGray, COLOR_BGR2GRAY);
    cv::GaussianBlur(roiRLGray, roiRLGray, Size(3, 3), 0);
    int invFlag;
    int threshValue;

    if (ui->comboBoxCaliColorInv->currentText() == "No-Invert")
    {
        invFlag = THRESH_BINARY;
        threshValue = meanGS[0] + 60;
    }
    else
    {
        invFlag = THRESH_BINARY_INV;
        threshValue = meanGS[0] - 60;
    }

    cv::threshold(roiRLGray, thresholdImg, threshValue, 255, invFlag);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, Size(3,3));
    cv::erode(thresholdImg, thresholdImg, kernel);
    cv::dilate(thresholdImg, thresholdImg, kernel);
    //cv::imwrite("../images/grayT.jpg", thresholdImg);
}

void CalibratorForm::diffThreshold()
{
    cv::Mat src1, src2, res;
    cv::cvtColor(roiRLFrame, src1, COLOR_BGR2GRAY);
    cv::cvtColor(roiBGFrame, src2, COLOR_BGR2GRAY);
    cv::GaussianBlur(src1, src1, Size(3, 3), 0);
    cv::GaussianBlur(src2, src2, Size(3, 3), 0);
    cv::absdiff(src1, src2, res);
    cv::threshold(res, thresholdImg, 70, 255, THRESH_BINARY);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, Size(3,3));
    cv::erode(thresholdImg, thresholdImg, kernel);
    cv::dilate(thresholdImg, thresholdImg, kernel);
    //cv::imwrite("../images/test.jpg", thresholdImg);
}

void CalibratorForm::getContour()
{
    roiRLFrame2Show = roiRLFrame.clone();
    std::vector<vector<Point>> contours;
    std::vector<Vec4i> hierarchy;

    cv::findContours(thresholdImg, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

    std::vector<vector<Point> > contours_poly( contours.size() );
    std::vector<Point2f> center( contours.size() );
    std::vector<float> radius( contours.size() );

    for (size_t i = 0; i < contours.size(); i++)
    {
        cv::approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
        cv::minEnclosingCircle(contours_poly[i], center[i], radius[i]);
        double area = M_PI * radius[i] * radius[i];
        if (area > 20000)
        {
            pixelDistance = 2 * radius[i];
            //cv::drawContours(roiRLFrame2Show, contours, i, Scalar(255, 0, 0), CV_FILLED, 8, hierarchy);
            cv::circle(roiRLFrame2Show, center[i], radius[i], Scalar(0, 0, 255), 2, 8, 0 );
            cv::line(roiRLFrame2Show, Point(center[i].x-radius[i], center[i].y), Point(center[i].x+radius[i], center[i].y), Scalar(255, 0, 0), 2, 8);
            cv::circle(roiRLFrame2Show, center[i], 1, Scalar(0, 0, 255), 3, 8, 0 );
            // Create an output string stream
            std::ostringstream streamObj3;
            // Set Fixed -Point Notation
            streamObj3 << std::fixed;
            // Set precision to 1 digits
            streamObj3 << std::setprecision(1);
            //Add double to stream
            streamObj3 << pixelDistance;
            // Get string from output string stream
            std::string strObj3 = streamObj3.str();
            string printDistance = "D:" + strObj3 + "p";
            cv::putText(roiRLFrame2Show, printDistance, Point2f(center[i].x-100, center[i].y-20), 1, 3, Scalar(0, 255, 0), 3, 8);
        }
    }
    //cv::imwrite("../images/CT.jpg", roiRLFrame);
}

void CalibratorForm::on_pushButtonCalculate_clicked()
{
    if (ui->lineEditmmD->text().isEmpty())
    {
        QMessageBox::information(this, "No Input", "Please fill in real distance(float).");
    }
    else
    {
        QString mmDistanceStr = ui->lineEditmmD->text();
        double mmDistance = mmDistanceStr.toDouble();
        pixelPERmm = pixelDistance / mmDistance;
        ui->labelShowCalResult->setText(QString::number(pixelPERmm, 'f', 2));
    }
}

void CalibratorForm::receiveSetButtonVisible(QString input)
{
    if (input == "HSV" || input == "GrayScale" || input == "Diff")
    {
        if (input == "HSV" || input == "Diff")
        {
            ui->comboBoxCaliColorInv->setVisible(false);
        }
        else if (input == "GrayScale" && ui->comboBoxCaliMethod->currentText() == "Auto")
        {
            ui->comboBoxCaliColorInv->setVisible(true);
        }
    }
    else if (input == "Auto" || input == "Manual")
    {
        if (input == "Manual")
        {
            ui->comboBoxCaliColorInv->setVisible(false);
            ui->comboBoxCaliColor->setVisible(false);
        }
        else if (input == "Auto")
        {
            ui->comboBoxCaliColor->setVisible(true);
            if (ui->comboBoxCaliColor->currentText() == "GrayScale")
            {
                ui->comboBoxCaliColorInv->setVisible(true);
            }
            else
            {
                ui->comboBoxCaliColorInv->setVisible(false);
            }
        }
    }
}

void CalibratorForm::receiveCloseForm()
{
    this->close();
}
