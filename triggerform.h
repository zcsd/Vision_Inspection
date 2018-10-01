#ifndef TRIGGERFORM_H
#define TRIGGERFORM_H

#include <QWidget>
#include <QDebug>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"

using namespace cv;

namespace Ui {
class TriggerForm;
}

class TriggerForm : public QWidget
{
    Q_OBJECT

public:
    explicit TriggerForm(QWidget *parent = nullptr);
    ~TriggerForm();

private slots:
    void on_pushButtonStart_clicked();

    void on_pushButtonStop_clicked();

private:
    Ui::TriggerForm *ui;
    cv::VideoCapture capture;
    cv::Mat frame;
    void initUSBCam();
    void releaseUSBCam();
};

#endif // TRIGGERFORM_H
