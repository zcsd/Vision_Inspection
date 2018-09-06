// CalibratorForm class: Do all calibration steps
// Author: @ZC
// Date: created on 05 Sep 2018
#ifndef CALIBRATORFORM_H
#define CALIBRATORFORM_H

#include <QWidget>
#include <QDebug>

#include <unistd.h> // for usleep()

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
using namespace cv;

namespace Ui {
class CalibratorForm;
}

class CalibratorForm : public QWidget
{
    Q_OBJECT

public:
    explicit CalibratorForm(QWidget *parent = 0);
    ~CalibratorForm();

signals:
    void sendFrameRequest();

public slots:
    void receiveFrame(cv::Mat frame);

private slots:
    void on_pushButtonBGStart_clicked();
    void receiveCloseForm();

private:
    Ui::CalibratorForm *ui;
    cv::Mat frameCopy;
    void extractColorMean();

};

#endif // CALIBRATORFORM_H
