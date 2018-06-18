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
    delete ui;
    delete frameGrabber;
}

void MainWindow::initialSetup()
{
    ui->labelShowFrame->setStyleSheet("background-color: rgb(253, 253, 253);");
    // N means No Grabbing
    grabMode = 'N';
    // Set button initial status and color
    ui->pushButtonConnect->setEnabled(false);
    ui->pushButtonDisconnect->setEnabled(false);
    ui->pushButtonCapture->setEnabled(false);
    ui->pushButtonStream->setEnabled(false);
    ui->pushButtonStop->setEnabled(false);
    ui->pushButtonScanDevices->setEnabled(true);
    ui->pushButtonStop->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonDisconnect->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonCapture->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonStream->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonScanDevices->setStyleSheet("background-color: rgb(225, 225, 225);");
    // Interation between UI and frameGrabber
    connect(this, SIGNAL(sendConnect()), frameGrabber, SLOT(receiveConnectCamera()));
    connect(this, SIGNAL(sendDisconnect()), frameGrabber, SLOT(receiveDisconnectCamera()));
    connect(this, SIGNAL(sendCaptureMode()), frameGrabber, SLOT(receiveStartCaptureMode()));
    connect(this, SIGNAL(sendStreamMode()), frameGrabber, SLOT(receiveStartStreamMode()));
    connect(this, SIGNAL(sendStopGrabbing()), frameGrabber, SLOT(receiveStopGrabbing()));
    connect(frameGrabber, SIGNAL(sendCaptureFrame(QImage)), this, SLOT(receiveShowFrame(QImage)));

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
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Camera is close.");
        }
        else {
            ui->listWidgetMessageLog->addItem("[Error]   " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Fail to disconnect camera.");
            QMessageBox::information(this, "Disconnetion Failure", "Please Debug.");
        }
    }
    else if (grabMode == 'C' || grabMode == 'S') {
        QMessageBox::StandardButton disconnectReply;
        disconnectReply = QMessageBox::question(this, "Disconnect",
                                                "Camera is still in grabbing mode. Are you sure to disconnect camera?",
                                                QMessageBox::Yes|QMessageBox::No);
        if (disconnectReply == QMessageBox::Yes) {
            emit sendDisconnect();
            usleep(5000);
            if (!frameGrabber->cameraConnected) {
                on_pushButtonStop_clicked();
                ui->pushButtonCapture->setEnabled(false);
                ui->pushButtonCapture->setStyleSheet("background-color: rgb(225, 225, 225);");
                ui->pushButtonStream->setEnabled(false);
                ui->pushButtonStream->setStyleSheet("background-color: rgb(225, 225, 225);");
                ui->pushButtonDisconnect->setEnabled(false);
                ui->pushButtonConnect->setEnabled(true);
                ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);");
                ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Camera is close.");
            }
          }
    }
}

void MainWindow::on_pushButtonCapture_clicked()
{
    emit sendCaptureMode();

    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Capture one image.");
    grabMode = 'C';
    ui->pushButtonCapture->setStyleSheet("background-color: rgb(100, 255, 100);");
    ui->pushButtonStream->setEnabled(false);
    ui->pushButtonStop->setStyleSheet("background-color: rgb(255, 0, 0); color: rgb(255, 0, 0);");
    ui->pushButtonStop->setEnabled(true);
}

void MainWindow::on_pushButtonStream_clicked()
{  
    grabMode = 'S';
    ui->pushButtonStream->setEnabled(false);
    ui->pushButtonStream->setStyleSheet("background-color: rgb(100, 255, 100);");
    ui->pushButtonCapture->setEnabled(false);
    ui->pushButtonStop->setStyleSheet("background-color: rgb(255, 0, 0); color: rgb(255, 0, 0);");
    ui->pushButtonStop->setEnabled(true);

    emit sendStreamMode();

    streamTrigger = new QTimer();
    streamTrigger->setInterval(1);

    connect(streamTrigger, SIGNAL(timeout()), frameGrabber, SLOT(receiveSendFrame()));
    connect(frameGrabber, SIGNAL(sendFrame(QImage)), this, SLOT(receiveShowFrame(QImage)));

    streamTrigger->start();

    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Start streaming mode.");
}

void MainWindow::on_pushButtonStop_clicked()
{
    if ( grabMode == 'C' ) {
        emit sendStopGrabbing();
        usleep(5000);

        if (!frameGrabber->startGrabbing) {
            grabMode = 'N';
            ui->pushButtonCapture->setEnabled(true);
            ui->pushButtonCapture->setStyleSheet("background-color: rgb(225, 225, 225);");
            ui->pushButtonStream->setEnabled(true);
            ui->pushButtonStop->setStyleSheet("background-color: rgb(225, 225, 225);");
            ui->pushButtonStop->setEnabled(false);
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Quit streaming mode.");
        }
    }
    else if ( grabMode == 'S' ) {
        delete streamTrigger;
        emit sendStopGrabbing();
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

void MainWindow::receiveShowFrame(QImage qShowFrame)
{
    ui->labelShowFrame->setPixmap(QPixmap::fromImage(qShowFrame));
}

