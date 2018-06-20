#include "cameracalibrator.h"
#include "ui_cameracalibrator.h"

CameraCalibrator::CameraCalibrator(QWidget *parent) : QWidget(parent)
{
    Ui_Form *caliUI;
    caliUI = new Ui_Form();
    caliUI->setupUi(this);
    this->show();
}
