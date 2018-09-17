#include "calibratorform.h"
#include "ui_calibratorform.h"

CalibratorForm::CalibratorForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalibratorForm)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::WindowStaysOnTopHint); // always on top
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

    connect(ui->buttonBoxEnd, SIGNAL(rejected()), this, SLOT(receiveCancelForm()));
    connect(ui->buttonBoxEnd, SIGNAL(accepted()), this, SLOT(receiveOkForm()));
    // set button visible or not
    receiveSetButtonVisible(ui->comboBoxCaliMethod->currentText());
    connect(ui->comboBoxCaliMethod, SIGNAL(activated(QString)), this, SLOT(receiveSetButtonVisible(QString)));
    // @ZC, temp using!!! for step 1 and 2
    ROI = Rect(10, 150, 2428, 1600); // original 2448x2048, now 2428x1600
}

void CalibratorForm::receiveFrame(cv::Mat frame)
{
    frameCopy = frame.clone();
    // TODO @ZC, add second frame update check
    if (frameCopy.empty())
    {
        newFrameAvaviable = false;
    }
    else
    {
        newFrameAvaviable = true;
    }
}

void CalibratorForm::receiveMousePressedPosition(QPoint &pos)
{
    int x = pos.x();
    int y = pos.y();

    int x_topLeft, y_topLeft;
    // 2322 = 2448 - 125 -1
    if ((x/0.4 - 62.5) >= 2322 )
    {
        x_topLeft = 2322;
    }
    else if ((x/0.4 - 62.5) <= 1)
    {
        x_topLeft = 1;
    }
    else
    {
        x_topLeft = int(x/0.4 - 62.5);
    }

    if ((y/0.4 - 62.5) >= 1922 )
    {
        y_topLeft = 1922;
    }
    else if ((y/0.4 - 62.5) <= 1)
    {
        y_topLeft = 1;
    }
    else
    {
        y_topLeft = int(y/0.4 - 62.5);
    }

    cv::Rect objROI = Rect(x_topLeft, y_topLeft, 125, 125);
    if (newFrameAvaviable)
    {
        cv::Mat objImage = frameCopy(objROI).clone();
        extractObjColorMean(objImage);
    }
}

void CalibratorForm::on_pushButtonBGStart_clicked()
{
    emit sendFrameRequest();
    usleep(2000);

    if (newFrameAvaviable)
    {
        extractBGColorMean();
    }
    else
    {
        QMessageBox::warning(this, "No Image Captured", "Please capture correct image.");
    }
}

void CalibratorForm::extractBGColorMean()
{
    roiBGFrame = frameCopy(ROI).clone();

    cv::Mat roiFrameHSV, roiFrameGS;
    cv::cvtColor(roiBGFrame, roiFrameHSV, COLOR_BGR2HSV_FULL);
    cv::cvtColor(roiBGFrame, roiFrameGS, COLOR_BGR2GRAY);
    bgMeanGS = cv::mean(roiFrameGS);
    bgMeanBGR = cv::mean(roiBGFrame); // B G R order
    bgMeanHSV = cv::mean(roiFrameHSV); // H S V order
    QString grayVauleStr = QString::number(int(bgMeanGS[0]));
    QString bgrValueStr = QString::number(int(bgMeanBGR[0])) + "," + QString::number(int(bgMeanBGR[1]))
                          + "," + QString::number(int(bgMeanBGR[2]));
    QString hsvValueStr = QString::number(int(bgMeanHSV[0])) + "," + QString::number(int(bgMeanHSV[1]))
                          + "," + QString::number(int(bgMeanHSV[2]));
    QString rgbValueSS =  "background-color: rgb(" + QString::number(int(bgMeanBGR[2])) + ", "
                          + QString::number(int(bgMeanBGR[1])) + ", " + QString::number(int(bgMeanBGR[0])) + ");";
    ui->labelShowRGB->setText(bgrValueStr);
    ui->labelShowHSV->setText(hsvValueStr);
    ui->labelShowGS->setText(grayVauleStr);
    ui->labelDisplayColor->setStyleSheet(rgbValueSS);
}

void CalibratorForm::extractObjColorMean(cv::Mat image)
{
    cv::imwrite("../images/testa.jpg", image);
    cv::Mat imageHSV, imageGS;
    cv::cvtColor(image, imageHSV, COLOR_BGR2HSV_FULL);
    cv::cvtColor(image, imageGS, COLOR_BGR2GRAY);
    objMeanGS = cv::mean(imageGS);
    objMeanBGR = cv::mean(image); // B G R order
    objMeanHSV = cv::mean(imageHSV); // H S V order
    QString grayVauleStr = QString::number(int(objMeanGS[0]));
    QString bgrValueStr = QString::number(int(objMeanBGR[0])) + "," + QString::number(int(objMeanBGR[1]))
                          + "," + QString::number(int(objMeanBGR[2]));
    QString hsvValueStr = QString::number(int(objMeanHSV[0])) + "," + QString::number(int(objMeanHSV[1]))
                          + "," + QString::number(int(objMeanHSV[2]));
    QString rgbValueSS =  "background-color: rgb(" + QString::number(int(objMeanBGR[2])) + ", "
                          + QString::number(int(objMeanBGR[1])) + ", " + QString::number(int(objMeanBGR[0])) + ");";
    ui->labelShowObjRGB->setText(bgrValueStr);
    ui->labelShowObjHSV->setText(hsvValueStr);
    ui->labelShowObjGS->setText(grayVauleStr);
    ui->labelDisplayObjColor->setStyleSheet(rgbValueSS);
}

void CalibratorForm::on_pushButtonRulerStart_clicked()
{
    if (newFrameAvaviable)
    {
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
    else
    {
        QMessageBox::warning(this, "No Image Captured", "Please capture correct image.");
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
    int h = bgMeanHSV[0], s = bgMeanHSV[1], v = bgMeanHSV[2];
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

    bool invert = false;

    if (bgMeanGS[0] > objMeanGS[0])
    {
        invert = true;
    }
    else
    {
        invert = false;
    }

    if (invert)
    {
        invFlag = THRESH_BINARY_INV;
        threshValue = bgMeanGS[0] - int(abs(bgMeanGS[0]-objMeanGS[0])/2);
    }
    else
    {

        invFlag = THRESH_BINARY;
        threshValue = bgMeanGS[0] + int(abs(bgMeanGS[0]-objMeanGS[0])/2);
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
    if (ui->lineEditmmD->text().isEmpty() || !ui->labelShowPixelD->text().contains('.'))
    {
        QMessageBox::warning(this, "Wrong Input", "Please check calibration steps.");
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
    if (input == "Auto" || input == "Manual")
    {
        if (input == "Manual")
        {
            ui->comboBoxCaliColor->setVisible(false);
        }
        else if (input == "Auto")
        {
            ui->comboBoxCaliColor->setVisible(true);
        }
    }
}

void CalibratorForm::receiveCancelForm()
{
    this->close();
    //this->~CalibratorForm();
}

void CalibratorForm::receiveOkForm()
{
    if (ui->labelShowCalResult->text().contains('.'))
    {
        QMessageBox::StandardButton updateConfReply;
        updateConfReply = QMessageBox::question(this, "Update Configuration",
                                                "Yes to update configuration file, No to Cancel.",
                                                QMessageBox::Yes|QMessageBox::No);
        if (updateConfReply == QMessageBox::Yes)
        {
            writeCaliConf();
            emit sendUpdateConfig();
            receiveCancelForm();
        }
    }
    else
    {
        QMessageBox::warning(this,"No Calculated Result","Please complete calibration steps.",QMessageBox::Yes);
    }
}

void CalibratorForm::writeCaliConf()
{
    QFile caliConfFile("../conf/calibration.conf");
    if(!caliConfFile.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
    {
         QMessageBox::warning(this,"File Write Error","Configuration file can't open",QMessageBox::Yes);
    }
    else
    {
        QTextStream in(&caliConfFile);
        in << QString::number(pixelPERmm, 'f', 2) << "\n";
        caliConfFile.close();
    }
}

void CalibratorForm::on_pushButtonRulerSelect_clicked()
{
    emit sendFrameRequest(); // require 2nd frame for step 2
    usleep(2000);

    emit sendCaliCommand("SelectStart");
}

void CalibratorForm::on_pushButtonRulerSelectStop_clicked()
{
    emit sendCaliCommand("SelectStop");
}
