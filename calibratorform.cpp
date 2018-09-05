#include "calibratorform.h"
#include "ui_calibratorform.h"

CalibratorForm::CalibratorForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalibratorForm)
{
    ui->setupUi(this);
}

CalibratorForm::~CalibratorForm()
{
    delete ui;
}

void CalibratorForm::on_pushButton_clicked()
{
    qDebug() << "I am in calibrator";
    emit sendTest();
}
