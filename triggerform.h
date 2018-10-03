#ifndef TRIGGERFORM_H
#define TRIGGERFORM_H

#include <QWidget>
#include <QDebug>
#include <QTimer>
#include <QTime>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
using namespace cv;

#include <stdio.h>
using namespace std;

namespace Ui {
class TriggerForm;
}

class TriggerForm : public QWidget
{
    Q_OBJECT

public:
    explicit TriggerForm(QWidget *parent = nullptr);
    ~TriggerForm();

signals:
    void sendTrigger();

private slots:
    void on_pushButtonStart_clicked();
    void on_pushButtonStop_clicked();
    void receiveUpdateFrame();

    void on_pushButtonReset_clicked();

private:
    Ui::TriggerForm *ui;
    cv::VideoCapture capture;
    QPixmap whitePixmap;
    cv::Mat bgImg;
    int partsCounter = 0;
    int frameCounter = 0;
    bool startCount = false;
    QTimer *camTrigger;
    void initUSBCam();
    void releaseUSBCam();
    cv::Mat processFrame(cv::Mat img);
};

#endif // TRIGGERFORM_H
