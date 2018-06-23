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
    setMCaliVisible(false);

    QPixmap whiteBackground = QPixmap(1024, 786);
    whiteBackground.fill(Qt::white);
    ui->labelShowFrame->setPixmap(whiteBackground);
    // N means No Grabbing
    grabMode = 'N';
    ui->labelCalResult->setAlignment(Qt::AlignCenter);
    ui->labelShowPos->setAlignment(Qt::AlignCenter);
    ui->labelShowRes->setAlignment(Qt::AlignCenter);
    ui->labelShowScale->setAlignment(Qt::AlignCenter);
    ui->labelShowRGB->setAlignment(Qt::AlignCenter);
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
    readCaliConf();
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
    ui->labelShowRes->setText("");
    ui->labelShowPos->setText("");
    ui->labelShowRGB->setText("");
    ui->labelShowScale->setText("");
}

void MainWindow::on_pushButtonCapture_clicked()
{
    emit sendCaptureMode();
    ui->labelShowRes->setText("2048x1536");
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");

    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Capture one image.");
    grabMode = 'C';
    ui->pushButtonStream->setEnabled(true);
    ui->pushButtonStop->setEnabled(false);
    ui->pushButtonSaveCapture->setEnabled(true);
    ui->labelShowFrame->setMouseTracking(true);
}

void MainWindow::on_pushButtonSaveCapture_clicked()
{
    QString filePath = defaultSavePath + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".bmp";
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
    ui->labelShowRes->setText("2048x1536");
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");

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
            ui->labelShowRes->setText("");
            ui->labelShowPos->setText("");
            ui->labelShowRGB->setText("");
            ui->labelShowScale->setText("");
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
    cv::Mat tempFrame;
    if (autoCalibration) {
        tempFrame = frameToCali.clone();
    } else {
        tempFrame = cvRawFrameCopy.clone();
    }
    cv::resize(tempFrame, cvRGBFrame, cv::Size(), scaleFactor, scaleFactor);
    cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
    qDisplayedFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
    ui->labelShowFrame->setPixmap(QPixmap::fromImage(qDisplayedFrame));
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
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    if (grabMode == 'C')  displayFrame();
}

void MainWindow::on_actionZoomToFit_triggered()
{
    scaleFactor = 0.5;
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Scaling factor: " + QString::number(scaleFactor));
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
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
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    if (grabMode == 'C')  displayFrame();
}

void MainWindow::on_actionZoomToRaw_triggered()
{
    scaleFactor = 1.0;
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Scaling factor: " + QString::number(scaleFactor));
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    if (grabMode == 'C')  displayFrame();
}

void MainWindow::receiveShowMousePosition(QPoint &pos)
{
    ui->labelShowPos->setText(QString::number(pos.x()) + ", " + QString::number((pos.y())) );
}

void MainWindow::on_actionMCalibrate_triggered()
{
    manualCalibration = true;
    autoCalibration = false;
    ui->pushButtonStartCali->setEnabled(true);
    ui->pushButtonRedoCali->setEnabled(false);
    ui->pushButtonConfirm->setEnabled(false);
    ui->pushButtonCalculate->setEnabled(false);
    ui->lineEditRealDistance->setEnabled(false);
    ui->labelRealDisName->setEnabled(false);
    ui->labelCalResult->setEnabled(false);
    setMCaliVisible(true);
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Manual calibration selected.");
}

void MainWindow::on_actionOpenImage_triggered()
{
    grabMode = 'C';
    ui->labelShowRes->setText("2048x1536");
    ui->labelShowFrame->setMouseTracking(true);
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    QString openFilePath = QFileDialog::getOpenFileName(this, "Open an image", "../");
    QByteArray ba = openFilePath.toLatin1();
    const char *imagePath = ba.data();
    if (openFilePath.isEmpty())
        imagePath = "../images/c1.bmp";
    cv::Mat openImage = imread(imagePath, 1);
    receiveRawFrame(openImage);
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Opened an image.");
}

void MainWindow::on_pushButtonStartCali_clicked()
{
    if (manualCalibration) {
        ui->labelShowFrame->startMCalibration();
        ui->pushButtonRedoCali->setEnabled(true);
        ui->pushButtonStartCali->setEnabled(false);
        ui->pushButtonCalculate->setEnabled(true);
        ui->lineEditRealDistance->setEnabled(true);
        ui->labelRealDisName->setEnabled(true);
        ui->labelCalResult->setEnabled(true);
        ui->pushButtonConfirm->setEnabled(false);
        ui->lineEditRealDistance->clear();
        ui->labelCalResult->clear();
        QDoubleValidator *validator = new QDoubleValidator(0.00, 9999.99, 2);;
        ui->lineEditRealDistance->setValidator(validator);
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "     Manual calibration started.");
    }
    if (autoCalibration) {
        ui->pushButtonRedoCali->setEnabled(true);
        ui->pushButtonStartCali->setEnabled(false);
        ui->pushButtonCalculate->setEnabled(true);
        ui->lineEditRealDistance->setEnabled(true);
        ui->labelRealDisName->setEnabled(true);
        ui->labelCalResult->setEnabled(true);
        ui->pushButtonConfirm->setEnabled(false);
        ui->lineEditRealDistance->clear();
        ui->labelCalResult->clear();
        QDoubleValidator *validator = new QDoubleValidator(0.00, 9999.99, 2);;
        ui->lineEditRealDistance->setValidator(validator);
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Auto Calibration started.");

        frameToCali = cvRawFrameCopy.clone();
        RulerCalibrator rulerCalibrator(frameToCali, pixelDistanceAC);
        cv::resize(frameToCali, cvRGBFrame, cv::Size(), scaleFactor, scaleFactor);
        cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
        qDisplayedFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
        ui->labelShowFrame->setPixmap(QPixmap::fromImage(qDisplayedFrame));
    }
}

void MainWindow::on_pushButtonRedoCali_clicked()
{
    if (manualCalibration) {
        ui->labelShowFrame->redoMCalibration();
        ui->pushButtonRedoCali->setEnabled(true);
        ui->pushButtonStartCali->setEnabled(false);
        ui->pushButtonCalculate->setEnabled(true);
        ui->lineEditRealDistance->setEnabled(true);
        ui->labelRealDisName->setEnabled(true);
        ui->labelCalResult->setEnabled(true);
        ui->pushButtonConfirm->setEnabled(false);
        ui->lineEditRealDistance->clear();
        ui->labelCalResult->clear();
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Manual calibration restarted.");
    }
    if (autoCalibration) {
        ui->pushButtonRedoCali->setEnabled(true);
        ui->pushButtonStartCali->setEnabled(false);
        ui->pushButtonCalculate->setEnabled(true);
        ui->lineEditRealDistance->setEnabled(true);
        ui->labelRealDisName->setEnabled(true);
        ui->labelCalResult->setEnabled(true);
        ui->pushButtonConfirm->setEnabled(false);
        ui->lineEditRealDistance->clear();
        ui->labelCalResult->clear();
        QDoubleValidator *validator = new QDoubleValidator(0.00, 9999.99, 2);;
        ui->lineEditRealDistance->setValidator(validator);
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Auto Calibration started.");

        frameToCali = cvRawFrameCopy.clone();
        RulerCalibrator rulerCalibrator(frameToCali, pixelDistanceAC);
        cv::resize(frameToCali, cvRGBFrame, cv::Size(), scaleFactor, scaleFactor);
        cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
        qDisplayedFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
        ui->labelShowFrame->setPixmap(QPixmap::fromImage(qDisplayedFrame));
    }
}

void MainWindow::on_pushButtonCalculate_clicked()
{
    if (manualCalibration) {
        if (ui->lineEditRealDistance->text().isEmpty()) {
            QMessageBox::information(this, "No Input", "Please fill in number only.");
        } else {
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Calculate calibration results.");
            QString  distance = ui->lineEditRealDistance->text();
            double realDistance = distance.toDouble();
            double pixelDistance = ui->labelShowFrame->getAverageDistance();
            pixelPerMM = pixelDistance / realDistance;
            ui->labelCalResult->setText(QString::number(pixelPerMM, 'f', 2) + " pixel/mm");
            ui->pushButtonConfirm->setEnabled(true);
        }
    }
    if (autoCalibration) {
        if (ui->lineEditRealDistance->text().isEmpty()) {
            QMessageBox::information(this, "No Input", "Please fill in number only.");
        } else {
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Calculate calibration results.");
            QString  distance = ui->lineEditRealDistance->text();
            double realDistance = distance.toDouble();
            pixelPerMM = pixelDistanceAC / realDistance;
            ui->labelCalResult->setText(QString::number(pixelPerMM, 'f', 2) + " pixel/mm");
            ui->pushButtonConfirm->setEnabled(true);
        }
    }
}

void MainWindow::writeCaliConf()
{
    QFile caliConfFile("../conf/calibration.conf");
    if(!caliConfFile.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate)) {
         QMessageBox::warning(this,"File Write Error","Conf file can't open",QMessageBox::Yes);
    } else {
        QTextStream in(&caliConfFile);
        in << QString::number(pixelPerMM, 'f', 2) << "\n";
    }
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Calibration results updated to conf file.");
    caliConfFile.close();
}

void MainWindow::readCaliConf()
{
    QFile caliConfFile("../conf/calibration.conf");

    if(!caliConfFile.open(QIODevice::ReadOnly)) {
         QMessageBox::warning(this,"File Write Error","Conf file can't open",QMessageBox::Yes);
    } else {
        QTextStream in(&caliConfFile);
        QString line;
        line = in.readLine();
        currentPPMM = line.toDouble();
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Loaded configuration: " + line + " pixel/mm");
    }

    caliConfFile.close();
}

void MainWindow::on_pushButtonConfirm_clicked()
{
    if (manualCalibration || autoCalibration) {
        ui->labelShowFrame->finishMCalibration();
        setMCaliVisible(false);
        //ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Confirm calibration results.");
        QMessageBox::StandardButton updateConfReply;
        updateConfReply = QMessageBox::question(this, "Update Configuration",
                                                "Do you want to update configuration file?",
                                                QMessageBox::Yes|QMessageBox::No);
        if (updateConfReply == QMessageBox::Yes) {
            writeCaliConf();
            currentPPMM = pixelPerMM;
        }
    }
    manualCalibration = false;
    autoCalibration = false;
}

void MainWindow::setMCaliVisible(bool showMCali)
{
    ui->pushButtonStartCali->setVisible(showMCali);
    ui->pushButtonRedoCali->setVisible(showMCali);
    ui->pushButtonConfirm->setVisible(showMCali);
    ui->pushButtonCalculate->setVisible(showMCali);
    ui->lineEditRealDistance->setVisible(showMCali);
    ui->labelRealDisName->setVisible(showMCali);
    ui->labelCalResult->setVisible(showMCali);
    ui->labelMCaliName->setVisible(showMCali);
}

void MainWindow::on_actionACalibrate_triggered()
{
    manualCalibration = false;
    autoCalibration = true;
    ui->pushButtonStartCali->setEnabled(true);
    ui->pushButtonRedoCali->setEnabled(false);
    ui->pushButtonConfirm->setEnabled(false);
    ui->pushButtonCalculate->setEnabled(false);
    ui->lineEditRealDistance->setEnabled(false);
    ui->labelRealDisName->setEnabled(false);
    ui->labelCalResult->setEnabled(false);
    setMCaliVisible(true);
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "    Auto calibration selected.");

}
