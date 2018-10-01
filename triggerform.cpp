#include "triggerform.h"
#include "ui_triggerform.h"

TriggerForm::TriggerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TriggerForm)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window); // always on top
    initUSBCam();
}

TriggerForm::~TriggerForm()
{
    delete ui;
}

void TriggerForm::initUSBCam()
{
    capture.open(0);
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

    if(capture.isOpened())
    {
        qDebug() << "USB webcam is open.";
    }
    else
    {
        qDebug() << "Fail to open USB webcam.";
    }
}

void TriggerForm::releaseUSBCam()
{
    if(capture.isOpened())
    {
        capture.release();
        qDebug() << "USB webcam is released.";
    }
}

void TriggerForm::on_pushButtonStart_clicked()
{
    if(!capture.isOpened())
    {
        initUSBCam();
    }

}

void TriggerForm::on_pushButtonStop_clicked()
{
    releaseUSBCam();
}
