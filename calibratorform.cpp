#include "calibratorform.h"
#include "ui_calibratorform.h"

CalibratorForm::CalibratorForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalibratorForm)
{
    ui->setupUi(this);
    connect(ui->buttonBoxEnd, SIGNAL(rejected()), this, SLOT(receiveCloseForm()));
    // set color-invert button visible or not
    receiveSetColorInv(ui->comboBoxCaliColor->currentText());
    connect(ui->comboBoxCaliColor, SIGNAL(activated(QString)), this, SLOT(receiveSetColorInv(QString)));
    connect(ui->comboBoxCaliMethod, SIGNAL(activated(QString)), this, SLOT(receiveSetColorInv(QString)));
    // @ZC, temp using!!! for step 1 and 2
    ROI = Rect(10, 150, 2428, 1600); // original 2448x2048, now 2428x1600
}

CalibratorForm::~CalibratorForm()
{
    delete ui;
}

void CalibratorForm::receiveCloseForm()
{
    qDebug() << "Close Calibrator Form in CaliForm";
    this->close();
}

void CalibratorForm::receiveFrame(cv::Mat frame)
{
    qDebug() << "Receive Frame in CaliForm";
    frameCopy = frame.clone();
}

void CalibratorForm::on_pushButtonBGStart_clicked()
{
    qDebug() << "Step 1 Start.";
    emit sendFrameRequest();
    usleep(2000);
    extractColorMean();
}

void CalibratorForm::extractColorMean()
{
    cv::Mat roiBGFrame = frameCopy(ROI);

    cv::Mat roiFrameHSV;
    cvtColor(roiBGFrame, roiFrameHSV, COLOR_BGR2HSV_FULL);
    cv::Mat roiFrameGS;
    cv::cvtColor(roiBGFrame, roiFrameGS, COLOR_BGR2GRAY);
    meanGS = cv::mean(roiFrameGS);
    meanBGR = cv::mean(roiBGFrame); // B G R order
    meanHSV = cv::mean(roiFrameHSV); // H S V order
    qDebug() << meanGS[0] << meanGS[1] << meanGS[2];
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
}

void CalibratorForm::manualCalibrateRuler()
{

}

void CalibratorForm::hsvThreshold()
{
    cv::Mat roiRLFrame = frameCopy(ROI);
    cv::Mat roiRLHSV, thresholdHSV, thresholdImg;
    cv::GaussianBlur(roiRLFrame, roiRLFrame, Size(3, 3), 0);
    cv::cvtColor(roiRLFrame, roiRLHSV, COLOR_BGR2HSV_FULL);
    int h = meanHSV[0], s = meanHSV[1], v = meanHSV[2];
    cv::inRange(roiRLHSV, Scalar(h-20, s-100, v-100), Scalar(h+20, s+100, v+100), thresholdHSV); //48, 52 ,213//+-10,+-30,+-50
    //std::cout << thresholdHSV.type() << endl;
    cv::bitwise_not(thresholdHSV, thresholdImg);
    //cv::invert(thresholdHSV, thresholdHSVInv);
    // kernel shape: MORPH_RECT   MORPH_CROSS  MORPH_ELLIPSE
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, Size(3,3));
    cv::erode(thresholdImg, thresholdImg, kernel);
    cv::dilate(thresholdImg, thresholdImg, kernel);
    cv::imwrite("../images/hsvT.jpg", thresholdImg);
}

void CalibratorForm::grayscaleThreshold()
{
    cv::Mat roiRLFrame = frameCopy(ROI);
    cv::Mat roiRLGray, thresholdImg;
    cv::cvtColor(roiRLFrame, roiRLGray, COLOR_BGR2GRAY);
    cv::GaussianBlur(roiRLGray, roiRLGray, Size(3, 3), 0);
    int invFlag;

    if (ui->comboBoxCaliColorInv->currentText() == "Invert")
    {
        invFlag = THRESH_BINARY_INV;
    }
    else
    {
        invFlag = THRESH_BINARY;
    }

    cv::threshold(roiRLGray, thresholdImg, meanGS[0]-60, 255, invFlag);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, Size(3,3));
    cv::erode(thresholdImg, thresholdImg, kernel);
    cv::dilate(thresholdImg, thresholdImg, kernel);
    cv::imwrite("../images/grayT.jpg", thresholdImg);
}

void CalibratorForm::on_pushButtonRulerStart_clicked()
{
    qDebug() << "Step 2 Start.";
    emit sendFrameRequest(); // require 2nd frame for step 2
    usleep(2000);

    if (ui->comboBoxCaliMethod->currentText() == "Auto")
    {
        autoCalibrateRuler();
    }
    else if (ui->comboBoxCaliMethod->currentText() == "Manual")
    {
        manualCalibrateRuler();
    }
}

void CalibratorForm::receiveSetColorInv(QString input)
{
    if (input == "HSV" || input == "GrayScale")
    {
        if (input == "HSV")
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
        }
        else if (input == "Auto" && ui->comboBoxCaliColor->currentText() == "GrayScale")
        {
            ui->comboBoxCaliColorInv->setVisible(true);
        }
    }
}
