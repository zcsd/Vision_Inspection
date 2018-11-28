#include "triggerform.h"
#include "ui_triggerform.h"

TriggerForm::TriggerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TriggerForm)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window); // always on top

    whitePixmap= QPixmap(640, 480);
    whitePixmap.fill(Qt::white);

    if (!capture.isOpened())
    {
        ui->pushButtonStart->setEnabled(true);
        ui->pushButtonWork->setDisabled(true);
        ui->pushButtonStop->setDisabled(true);
    }

    connect(ui->listWidget->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            ui->listWidget, SLOT(scrollToBottom()));
}

TriggerForm::~TriggerForm()
{
    if(capture.isOpened())
    {
        releaseUSBCam();
    }
    delete workThread;
    delete camTrigger;
    delete ui;
}

void TriggerForm::initUSBCam()
{
    capture.open(0);
    capture.set(CAP_PROP_FRAME_WIDTH, 640);
    capture.set(CAP_PROP_FRAME_HEIGHT, 480);

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
    cv::Mat imgROI, grayImgROI;
    imgROI = img(ROI).clone();

    cv::cvtColor(imgROI, grayImgROI, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(grayImgROI, grayImgROI, Size(3,3), 0);

    cv::Mat resImg, thresholdImg;
    cv::absdiff(grayImgROI, bgImgROI, resImg);
    cv::threshold(resImg, thresholdImg, 20, 255, THRESH_BINARY);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    cv::findContours(thresholdImg, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

    if (startCount)
    {
        frameCounter++;
    }
    else
    {
        frameCounter = 0;
    }
    // skip how many frame (30fps)
    if (frameCounter >= 90)
    {
        frameCounter = 0;
        startCount = false;
    }

    for (size_t i = 0; i < contours.size(); i++)
    {
        double area = cv::contourArea(contours[i]);
        if (area >= 100)
        {
            if (area > 5000 && !startCount)
            {
                cv::drawContours(img(ROI), contours, int(i), Scalar(255, 0, 0), -1, 8, vector<Vec4i>(), 0, Point(0,0));
                startCount = true;
                emit sendTrigger();
                partsCounter ++;
                ui->lcdNumberCounter->display(partsCounter);
                ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                                  + "    " + "[" + QString::number(partsCounter) + "]Trigger sent.");
            }
        }
    }

    return img;
}

void TriggerForm::on_pushButtonStart_clicked()
{
    initUSBCam();
    if (!capture.isOpened())
    {
        initUSBCam();
    }
    else
    {
        ui->pushButtonWork->setEnabled(true);
        ui->pushButtonStart->setDisabled(true);
        ui->pushButtonStop->setEnabled(true);
        ui->pushButtonStart->setStyleSheet("background-color: rgb(100, 255, 100);");
    }

    workThread = new QThread(this);
    workThread->start();

    camTrigger = new QTimer(); // use timer to loop usb webcam stream
    camTrigger->setInterval(5);

    connect(camTrigger, SIGNAL(timeout()), this, SLOT(receiveUpdateFrame()), Qt::DirectConnection);
    connect(workThread, SIGNAL(finished()), camTrigger, SLOT(stop()));

    camTrigger->start(); // loop start
    camTrigger->moveToThread(workThread);
    ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Webcam is capturing.");
}

void TriggerForm::on_pushButtonStop_clicked()
{
    //camTrigger->stop();
    //delete camTrigger;
    workThread->quit();
    workThread->wait();

    ui->labelShowUSBFrame->setPixmap(whitePixmap);
    ui->pushButtonStart->setEnabled(true);
    ui->pushButtonStop->setDisabled(true);
    ui->pushButtonWork->setDisabled(true);
    ui->pushButtonStart->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonWork->setStyleSheet("background-color: rgb(225, 225, 225);");
    startWork = false;
    ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Trigger stop.");
    releaseUSBCam();
}

void TriggerForm::receiveUpdateFrame()
{
    cv::Mat processedImg;
    QImage qDisplayedFrame;
    capture >> frame;

    if(!frame.empty())
    {
        if (startWork)
        {
            processedImg = processFrame(frame);
        }
        else
        {
            processedImg = frame.clone();
        }

        cv::cvtColor(processedImg, processedImg, cv::COLOR_BGR2RGB);
        qDisplayedFrame = QImage((uchar*)processedImg.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        ui->labelShowUSBFrame->setPixmap(QPixmap::fromImage(qDisplayedFrame));
    }
}

void TriggerForm::on_pushButtonReset_clicked()
{
    partsCounter = 0;
    ui->lcdNumberCounter->display(partsCounter);
    ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Trigger counter reset.");
}

void TriggerForm::on_pushButtonWork_clicked()
{
    if (capture.isOpened())
    {
        cv::cvtColor(frame, bgImg, cv::COLOR_BGR2GRAY);
        ROI = Rect(120, 90, 400, 300);
        bgImgROI = bgImg(ROI).clone();
        startWork = true;
        ui->pushButtonWork->setStyleSheet("background-color: rgb(100, 255, 100);");
        ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    Trigger start to work.");
    }

}
