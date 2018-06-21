#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    frameGrabber = new FrameGrabber();
    initialSetup();
}

MainWindow::~MainWindow()
{
    delete frameGrabber;
    delete ui;
}

void MainWindow::initialSetup()
{
    ui->labelShowFrame->setScaledContents(true);
    ui->scrollArea->setBackgroundRole(QPalette::Dark);
    ui->scrollArea->setWidget(ui->labelShowFrame);
    ui->scrollArea->setVisible(true);

    QPixmap whiteBackground = QPixmap(1024, 786);
    whiteBackground.fill(Qt::white);
    ui->labelShowFrame->setPixmap(whiteBackground);
    // N means No Grabbing
    grabMode = 'N';
    // Set button initial status and color
    ui->pushButtonConnect->setEnabled(false);
    ui->pushButtonDisconnect->setEnabled(false);
    ui->pushButtonCapture->setEnabled(false);
    ui->pushButtonStream->setEnabled(false);
    ui->pushButtonStop->setEnabled(false);
    ui->pushButtonSaveCapture->setEnabled(false);
    ui->pushButtonScanDevices->setEnabled(true);
    ui->pushButtonStop->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonDisconnect->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonCapture->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonStream->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonScanDevices->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonSaveCapture->setStyleSheet("background-color: rgb(225, 225, 225);");
    // Interation between UI and frameGrabber
    connect(ui->listWidgetMessageLog->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), ui->listWidgetMessageLog, SLOT(scrollToBottom()));
    connect(this, SIGNAL(sendConnect()), frameGrabber, SLOT(receiveConnectCamera()));
    connect(this, SIGNAL(sendDisconnect()), frameGrabber, SLOT(receiveDisconnectCamera()));
    connect(this, SIGNAL(sendCaptureMode()), frameGrabber, SLOT(receiveStartCaptureMode()));
    connect(this, SIGNAL(sendStreamMode()), frameGrabber, SLOT(receiveStartStreamMode()));
    connect(this, SIGNAL(sendStopGrabbing()), frameGrabber, SLOT(receiveStopGrabbing()));
    connect(frameGrabber, SIGNAL(sendCaptureFrame(cv::Mat)), this, SLOT(receiveRawFrame(cv::Mat)));
    connect(ui->labelShowFrame, SIGNAL(sendMousePosition(QPoint&)), this, SLOT(receiveShowMousePosition(QPoint&)));

    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    System started.");
}

void MainWindow::on_pushButtonConnect_clicked()
{
    emit sendConnect();
    usleep(5000); // 5ms

    if (frameGrabber->cameraConnected) {
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Camera is open.");

        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonConnect->setStyleSheet("background-color: rgb(100, 255, 100);");
        ui->pushButtonDisconnect->setEnabled(true);
        ui->pushButtonCapture->setEnabled(true);
        ui->pushButtonStream->setEnabled(true);
    }
    else {
        ui->listWidgetMessageLog->addItem("[Error]   " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Fail to connect camera.");
        QMessageBox::information(this, "Connetion Failure", "Please Check Camera and Debug.");
    }
}

void MainWindow::on_pushButtonDisconnect_clicked()
{

    if (grabMode == 'N') {
        emit sendDisconnect();
        usleep(5000);
        if (!frameGrabber->cameraConnected) {
            ui->pushButtonDisconnect->setEnabled(false);
            ui->pushButtonConnect->setEnabled(true);
            ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);");
            ui->pushButtonCapture->setEnabled(false);
            ui->pushButtonStream->setEnabled(false);
            ui->pushButtonSaveCapture->setEnabled(false);
            ui->pushButtonStop->setEnabled(false);
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Camera is close.");
        }
        else {
            ui->listWidgetMessageLog->addItem("[Error]   " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Fail to disconnect camera.");
            QMessageBox::information(this, "Disconnetion Failure", "Please Debug.");
        }
    }
    else if (grabMode == 'S') {
        QMessageBox::StandardButton disconnectReply;
        disconnectReply = QMessageBox::question(this, "Disconnect",
                                                "Camera is still in streaming mode. Are you sure to disconnect camera?",
                                                QMessageBox::Yes|QMessageBox::No);
        if (disconnectReply == QMessageBox::Yes) {
            emit sendDisconnect();
            usleep(5000);
            if (!frameGrabber->cameraConnected) {
                on_pushButtonStop_clicked();
                ui->pushButtonCapture->setEnabled(false);
                ui->pushButtonStream->setEnabled(false);
                ui->pushButtonStream->setStyleSheet("background-color: rgb(225, 225, 225);");
                ui->pushButtonDisconnect->setEnabled(false);
                ui->pushButtonSaveCapture->setEnabled(false);
                ui->pushButtonConnect->setEnabled(true);
                ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);");
                ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Camera is close.");
            }
          }
    }
    else if (grabMode == 'C') {
        emit sendDisconnect();
        usleep(5000);
        if (!frameGrabber->cameraConnected) {
            ui->pushButtonCapture->setEnabled(false);
            ui->pushButtonStream->setEnabled(false);
            ui->pushButtonStream->setStyleSheet("background-color: rgb(225, 225, 225);");
            ui->pushButtonDisconnect->setEnabled(false);
            ui->pushButtonSaveCapture->setEnabled(false);
            ui->pushButtonConnect->setEnabled(true);
            ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);");
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Camera is close.");
        }
    }
    ui->labelShowFrame->setMouseTracking(false);
    QPixmap whiteBackground = QPixmap(1024, 786);
    whiteBackground.fill(Qt::white);
    ui->labelShowFrame->setPixmap(whiteBackground);
}

void MainWindow::on_pushButtonCapture_clicked()
{
    emit sendCaptureMode();

    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Capture one image.");
    grabMode = 'C';
    ui->pushButtonStream->setEnabled(true);
    ui->pushButtonStop->setEnabled(false);
    ui->pushButtonSaveCapture->setEnabled(true);
    ui->labelShowFrame->setMouseTracking(true);
}

void MainWindow::on_pushButtonSaveCapture_clicked()
{
    QString filePath = defaultSavePath + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".jpg";
    QByteArray ba = filePath.toLatin1();
    const char *fileName = ba.data();

    if (cv::imwrite(fileName, cvRawFrameCopy)){
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Image saved.");
    }
}

void MainWindow::on_pushButtonStream_clicked()
{  
    grabMode = 'S';
    ui->pushButtonStream->setEnabled(false);
    ui->pushButtonStream->setStyleSheet("background-color: rgb(100, 255, 100);");
    ui->pushButtonCapture->setEnabled(false);
    ui->pushButtonStop->setStyleSheet("background-color: rgb(255, 0, 0); color: rgb(255, 0, 0);");
    ui->pushButtonStop->setEnabled(true);
    ui->pushButtonSaveCapture->setEnabled(false);

    emit sendStreamMode();

    streamTrigger = new QTimer();
    streamTrigger->setInterval(1);

    connect(streamTrigger, SIGNAL(timeout()), frameGrabber, SLOT(receiveSendFrame()));
    connect(frameGrabber, SIGNAL(sendFrame(cv::Mat)), this, SLOT(receiveRawFrame(cv::Mat)));

    streamTrigger->start();
    ui->labelShowFrame->setMouseTracking(true);

    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Start streaming mode.");
}

void MainWindow::on_pushButtonStop_clicked()
{
    if ( grabMode == 'S' ) {
        delete streamTrigger;
        emit sendStopGrabbing();
        ui->labelShowFrame->setMouseTracking(false);
        usleep(5000);

        if (!frameGrabber->startGrabbing) {
            grabMode = 'N';
            ui->pushButtonCapture->setEnabled(true);
            ui->pushButtonStream->setEnabled(true);
            ui->pushButtonStream->setStyleSheet("background-color: rgb(225, 225, 225);");
            ui->pushButtonStop->setStyleSheet("background-color: rgb(225 225, 225);");
            ui->pushButtonStop->setEnabled(false);
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Quit streaming mode.");
        }
    }
}

void MainWindow::on_pushButtonScanDevices_clicked()
{
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Scanning devices...");
    QString deviceName = frameGrabber->scanDevices();

    if (deviceName.contains("Basler", Qt::CaseSensitive)) {
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Camera found.");
        if (!frameGrabber->cameraConnected) {
            ui->pushButtonConnect->setEnabled(true);
        }
    }
    else {
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    No camera found, please check connection.");
        ui->pushButtonConnect->setEnabled(false);
    }

    ui->comboBoxDevices->clear();
    ui->comboBoxDevices->addItem(deviceName);
}

void MainWindow::receiveRawFrame(cv::Mat cvRawFrame)
{
    cvRawFrameCopy = cvRawFrame.clone();
    displayFrame();
}

void MainWindow::displayFrame()
{
    cv::resize(cvRawFrameCopy, cvRGBFrame, cv::Size(), scaleFactor, scaleFactor);
    cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
    qDisplayedFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
    ui->labelShowFrame->setPixmap(QPixmap::fromImage(qDisplayedFrame));
}

void MainWindow::on_pushButtonCalibrate_clicked()
{
    cameraCalibrator = new CameraCalibrator();
}

void MainWindow::on_actionChangeSavePath_triggered()
{
    defaultSavePath = QFileDialog::getExistingDirectory();
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Image save path changed to: " + defaultSavePath);
}

void MainWindow::on_actionZoomIn_triggered()
{
    if (scaleFactor >= 1.0) {
        scaleFactor += 1.0;
        if (scaleFactor > 5.0)  scaleFactor = 5.0;
    } else if (scaleFactor < 1.0) {
        scaleFactor *= 1.3333;
        if (scaleFactor > 0.4 && scaleFactor < 0.6)
            scaleFactor = 0.5;
        if (scaleFactor > 0.85 && scaleFactor < 1.1)
            scaleFactor = 1.0;
    }
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Scaling factor: " + QString::number(scaleFactor));
    if (grabMode == 'C')  displayFrame();
}

void MainWindow::on_actionZoomToFit_triggered()
{
    scaleFactor = 0.5;
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Scaling factor: " + QString::number(scaleFactor));
    if (grabMode == 'C')  displayFrame();
}

void MainWindow::on_actionZoomOut_triggered()
{
    // TODO: Scaling factor not smart, can not below 0.5, scaling method+
    if (scaleFactor > 1.0) {
        scaleFactor -= 1.0;
    } else if (scaleFactor <= 1.0) {
        scaleFactor *= 0.6667;
        if (scaleFactor > 0.4 && scaleFactor < 0.6)
            scaleFactor = 0.5;
        if (scaleFactor < 0.5)
            scaleFactor = 0.5;
    }
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Scaling factor: " + QString::number(scaleFactor));
    if (grabMode == 'C')  displayFrame();
}

void MainWindow::on_actionZoomToRaw_triggered()
{
    scaleFactor = 1.0;
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Scaling factor: " + QString::number(scaleFactor));
    if (grabMode == 'C')  displayFrame();
}

void MainWindow::receiveShowMousePosition(QPoint &pos)
{
    ui->labelShowPos->setAlignment(Qt::AlignCenter);
    ui->labelShowPos->setText(QString::number(pos.x()) + ", " + QString::number((pos.y())) );
}
