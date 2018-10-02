#include "triggerform.h"
#include "ui_triggerform.h"

TriggerForm::TriggerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TriggerForm)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window); // always on top
    initUSBCam();

    whitePixmap= QPixmap(640, 480);
    whitePixmap.fill(Qt::white);

    bgImg = cv::imread("../images/bg.png", 0);
    connect(ui->listWidget->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            ui->listWidget, SLOT(scrollToBottom()));
}

TriggerForm::~TriggerForm()
{
    if(capture.isOpened())
    {
        releaseUSBCam();
    }
    delete ui;
}

void TriggerForm::initUSBCam()
{
    capture.open(0);
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

    if(capture.isOpened())
    {
        ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    USB webcam is open.");
    }
    else
    {
        ui->listWidget->addItem("[ERROR]   " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    Fail to open USB webcam.");
    }
}

void TriggerForm::releaseUSBCam()
{
    if(capture.isOpened())
    {
        capture.release();
        ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    USB webcam is released.");
    }
}

Mat TriggerForm::processFrame(Mat img)
{
    cv::Mat grayImg;
    cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(grayImg, grayImg, Size(3,3), 0);

    cv::Mat resImg, thresholdImg;
    cv::absdiff(grayImg, bgImg, resImg);
    cv::threshold(resImg, thresholdImg, 20, 255, THRESH_BINARY);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    cv::findContours(thresholdImg, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if (area >= 100)
        {
            cv::drawContours(img, contours, int(i), Scalar(255, 0, 0), -1, 8, vector<Vec4i>(), 0, Point(0,0));
            if (area > 200)
            {
                emit sendTrigger();
            }
        }
    }

    return img;
}

void TriggerForm::on_pushButtonStart_clicked()
{
    if(!capture.isOpened())
    {
        initUSBCam();
    }

    camTrigger = new QTimer(); // use timer to loop usb webcam stream
    camTrigger->setInterval(1);
    connect(camTrigger, SIGNAL(timeout()), this, SLOT(receiveUpdateFrame()));

    camTrigger->start(); // loop start
    ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Trigger start to work.");
}

void TriggerForm::on_pushButtonStop_clicked()
{
    ui->labelShowUSBFrame->setPixmap(whitePixmap);
    camTrigger->stop();
    delete camTrigger;
    ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Trigger stop.");
    releaseUSBCam();
}

void TriggerForm::receiveUpdateFrame()
{
    cv::Mat frame, processedImg;
    QImage qDisplayedFrame;
    capture >> frame;
    //cv::imwrite("../images/bg.png", frame);
    if(!frame.empty())
    {
        processedImg = processFrame(frame);
        cv::cvtColor(processedImg, processedImg, cv::COLOR_BGR2RGB);
        qDisplayedFrame = QImage((uchar*)processedImg.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        ui->labelShowUSBFrame->setPixmap(QPixmap::fromImage(qDisplayedFrame));
    }
}
