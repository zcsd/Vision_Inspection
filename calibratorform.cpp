#include "calibratorform.h"
#include "ui_calibratorform.h"

CalibratorForm::CalibratorForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalibratorForm)
{
    ui->setupUi(this);
    connect(this->ui->pushButtonClose, SIGNAL(clicked()), this, SLOT(receiveCloseForm()));
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
    qDebug() << "BG Start in CaliForm";
    emit sendFrameRequest();
    usleep(2000);
    extractColorMean();
}

void CalibratorForm::extractColorMean()
{
    cv::Rect ROI = Rect(10, 10, 2428, 1938); // original 2448x2048, now 2428x1938
    cv::Mat roiFrame = frameCopy(ROI);

    cv::Mat roiFrameHSV;
    cvtColor(roiFrame, roiFrameHSV, COLOR_BGR2HSV_FULL);
    cv::Scalar meanBGR, meanHSV;
    meanBGR = cv::mean(roiFrame); // B G R order
    meanHSV = cv::mean(roiFrameHSV); // H S V order
    QString bgrValueStr = QString::number(int(meanBGR[0])) + "," + QString::number(int(meanBGR[1]))
                          + "," + QString::number(int(meanBGR[2]));
    QString hsvValueStr = QString::number(int(meanHSV[0])) + "," + QString::number(int(meanHSV[1]))
                          + "," + QString::number(int(meanHSV[2]));
    QString rgbValueSS =  "background-color: rgb(" + QString::number(int(meanBGR[2])) + ", "
                          + QString::number(int(meanBGR[1])) + ", " + QString::number(int(meanBGR[0])) + ");";
    ui->labelShowRGB->setText(bgrValueStr);
    ui->labelShowHSV->setText(hsvValueStr);
    ui->labelDisplayColor->setStyleSheet(rgbValueSS);
}
