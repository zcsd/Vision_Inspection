#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    frameGrabber = new FrameGrabber();
    pyClassification = new PyClassification();
    fdTester = new FDTester();

    initialSetup();
}

MainWindow::~MainWindow()
{
    delete pyClassification;
    delete fdTester;
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

    readCaliConf();

    bgImg.load("../resource/logo.png");
    ui->labelShowFrame->setPixmap(bgImg);

    ui->labelCalResult->setAlignment(Qt::AlignCenter);
    ui->labelShowPos->setAlignment(Qt::AlignCenter);
    ui->labelShowRes->setAlignment(Qt::AlignCenter);
    ui->labelShowScale->setAlignment(Qt::AlignCenter);
    ui->labelShowRGB->setAlignment(Qt::AlignCenter);

    ui->pushButtonConnect->setEnabled(false);
    ui->pushButtonDisconnect->setEnabled(false);
    ui->pushButtonCapture->setEnabled(false);
    ui->pushButtonStream->setEnabled(false);
    ui->pushButtonStop->setEnabled(false);
    ui->pushButtonSaveCapture->setEnabled(false);
    ui->pushButtonScanDevices->setEnabled(true);

    // Always dispaly the latest message in msg log in bottom
    connect(ui->listWidgetMessageLog->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            ui->listWidgetMessageLog, SLOT(scrollToBottom()));

    connect(this, SIGNAL(sendConnect()), frameGrabber, SLOT(receiveConnectCamera()));
    connect(this, SIGNAL(sendDisconnect()), frameGrabber, SLOT(receiveDisconnectCamera()));
    connect(this, SIGNAL(sendCaptureMode()), frameGrabber, SLOT(receiveStartCaptureMode()));
    connect(this, SIGNAL(sendStreamMode()), frameGrabber, SLOT(receiveStartStreamMode()));
    connect(this, SIGNAL(sendStopGrabbing()), frameGrabber, SLOT(receiveStopGrabbing()));
    connect(frameGrabber, SIGNAL(sendCaptureFrame(cv::Mat)), this, SLOT(receiveRawFrame(cv::Mat)));
    connect(ui->labelShowFrame, SIGNAL(sendMousePosition(QPoint&)), this, SLOT(receiveShowMousePosition(QPoint&)));

    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    System started.");
    ui->comboBoxMatchMethod->addItems({"Machine Learning", "Image Processing"});
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

void MainWindow::readCaliConf()
{
    QFile caliConfFile("../conf/calibration.conf");

    if (!caliConfFile.open(QIODevice::ReadOnly))
    {
         QMessageBox::warning(this,"File Write Error", "Conf file can't open", QMessageBox::Yes);
    }
    else
    {
        QTextStream in(&caliConfFile);
        QString line;
        line = in.readLine();
        currentPPMM = line.toDouble();
        caliConfFile.close();
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    Loaded ruler configuration: " + line + " pixel/mm");
    }  
}

void MainWindow::on_pushButtonConnect_clicked()
{
    emit sendConnect();
    usleep(2000); // 2ms

    if (frameGrabber->cameraConnected)
    {
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    Camera is open.");

        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonConnect->setStyleSheet("background-color: rgb(100, 255, 100);"); // Make this button green, it's open.
        ui->pushButtonDisconnect->setEnabled(true);
        ui->pushButtonCapture->setEnabled(true);
        ui->pushButtonStream->setEnabled(true);
    }
    else
    {
        ui->listWidgetMessageLog->addItem("[Error]   " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    Fail to connect camera.");
        QMessageBox::information(this, "Connetion Failure", "Please Check Camera and Debug.");
    }
}

void MainWindow::on_pushButtonDisconnect_clicked()
{
    // @ZC TODO: add one function to set disconnect status
    if (grabMode == 'N') // in camera close mode
    {
        emit sendDisconnect();
        usleep(5000);

        if (!frameGrabber->cameraConnected)
        {
            ui->pushButtonDisconnect->setEnabled(false);
            ui->pushButtonConnect->setEnabled(true);
            ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);"); // Make it gray color
            ui->pushButtonCapture->setEnabled(false);
            ui->pushButtonStream->setEnabled(false);
            ui->pushButtonSaveCapture->setEnabled(false);
            ui->pushButtonStop->setEnabled(false);
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                              + "    Camera is close.");
        }
        else
        {
            ui->listWidgetMessageLog->addItem("[Error]   " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                              + "    Fail to disconnect camera.");
            QMessageBox::information(this, "Disconnetion Failure", "Please Debug.");
        }
    }
    else if (grabMode == 'S') // in camera streaming mode
    {
        QMessageBox::StandardButton disconnectReply;
        disconnectReply = QMessageBox::question(this, "Disconnect",
                                                "Camera is still in streaming mode. Are you sure to disconnect camera?",
                                                QMessageBox::Yes|QMessageBox::No);
        if (disconnectReply == QMessageBox::Yes)
        {
            emit sendDisconnect();
            usleep(5000);

            if (!frameGrabber->cameraConnected)
            {
                on_pushButtonStop_clicked();
                ui->pushButtonCapture->setEnabled(false);
                ui->pushButtonStream->setEnabled(false);
                ui->pushButtonStream->setStyleSheet("background-color: rgb(225, 225, 225);");
                ui->pushButtonDisconnect->setEnabled(false);
                ui->pushButtonSaveCapture->setEnabled(false);
                ui->pushButtonConnect->setEnabled(true);
                ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);");
                ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                                  + "    Camera is close.");
            }
        }
    }
    else if (grabMode == 'C') // in camera capture mode
    {
        emit sendDisconnect();
        usleep(5000);

        if (!frameGrabber->cameraConnected)
        {
            ui->pushButtonCapture->setEnabled(false);
            ui->pushButtonStream->setEnabled(false);
            ui->pushButtonDisconnect->setEnabled(false);
            ui->pushButtonSaveCapture->setEnabled(false);
            ui->pushButtonConnect->setEnabled(true);
            ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);");
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                              + "    Camera is close.");
        }
    }

    ui->labelShowFrame->setMouseTracking(false);
    ui->labelShowFrame->setPixmap(bgImg);
    ui->labelShowRes->setText("");
    ui->labelShowPos->setText("");
    ui->labelShowRGB->setText("");
    ui->labelShowScale->setText("");
}

void MainWindow::on_pushButtonCapture_clicked()
{
    emit sendCaptureMode();
    grabMode = 'C';

    ui->pushButtonStream->setEnabled(true);
    ui->pushButtonStop->setEnabled(false);
    ui->pushButtonSaveCapture->setEnabled(true);
    ui->labelShowFrame->setMouseTracking(true);

    ui->labelShowRes->setText("2448x2048");
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Capture one image.");
}

void MainWindow::on_pushButtonSaveCapture_clicked()
{
    QString fileSavePathQStr = defaultSavePath + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".jpg";

    // convert QString to char
    QByteArray ba = fileSavePathQStr.toLatin1();
    const char *fileSavePathChar = ba.data();

    if (cv::imwrite(fileSavePathChar, cvRawFrameCopy))
    {
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    Image saved.");
    }
}

void MainWindow::on_pushButtonStream_clicked()
{  
    emit sendStreamMode();
    grabMode = 'S';

    ui->pushButtonStream->setEnabled(false);
    ui->pushButtonStream->setStyleSheet("background-color: rgb(100, 255, 100);"); // Make it green
    ui->pushButtonCapture->setEnabled(false);
    ui->pushButtonStop->setStyleSheet("background-color: rgb(255, 0, 0); color: rgb(255, 0, 0);"); // Make it red
    ui->pushButtonStop->setEnabled(true);
    ui->pushButtonSaveCapture->setEnabled(false);

    streamTrigger = new QTimer(); // use timer to loop streaming
    streamTrigger->setInterval(1);

    connect(streamTrigger, SIGNAL(timeout()), frameGrabber, SLOT(receiveSendFrame()));
    connect(frameGrabber, SIGNAL(sendFrame(cv::Mat)), this, SLOT(receiveRawFrame(cv::Mat)));

    streamTrigger->start(); // loop start

    ui->labelShowFrame->setMouseTracking(true);
    ui->labelShowRes->setText("2448x2048");
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Start streaming mode.");
}

void MainWindow::on_pushButtonStop_clicked()
{
    if (grabMode == 'S')
    {
        delete streamTrigger; // stop timer
        ui->labelShowFrame->setMouseTracking(false);
        emit sendStopGrabbing();
        usleep(5000);

        if (!frameGrabber->startGrabbing)
        {
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
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                              + "    Quit streaming mode.");
        }
    }
}

void MainWindow::on_pushButtonScanDevices_clicked()
{
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Scanning devices...");
    QString deviceName = frameGrabber->scanDevices();

    if (deviceName.contains("Basler", Qt::CaseSensitive))
    {
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    Basler Camera found.");
        if (!frameGrabber->cameraConnected)
        {
            ui->pushButtonConnect->setEnabled(true);
        }
    }
    else
    {
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    No camera found, please check connection.");
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
    if (autoCalibration)
    {
        tempFrame = frameToCali.clone();
    }
    else if (autoMeasure)
    {
        tempFrame = frameToMeasure.clone();
    }
    else
    {
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
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Image save path changed to: " + defaultSavePath);
}

void MainWindow::on_actionZoomIn_triggered()
{
    if (scaleFactor >= 1.0)
    {
        scaleFactor += 1.0;
        if (scaleFactor > 5.0)  scaleFactor = 5.0;
    }
    else if (scaleFactor < 1.0)
    {
        scaleFactor *= 1.3333;
        if (scaleFactor > 0.35 && scaleFactor < 0.5)  scaleFactor = 0.4;
        if (scaleFactor > 0.85 && scaleFactor < 1.1)  scaleFactor = 1.0;
    }

    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Scaling factor: " + QString::number(scaleFactor));
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    if (grabMode == 'C')  displayFrame();
}

void MainWindow::on_actionZoomToFit_triggered()
{
    scaleFactor = 0.4;
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Scaling factor: " + QString::number(scaleFactor));
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    if (grabMode == 'C')  displayFrame();
}

void MainWindow::on_actionZoomOut_triggered()
{
    if (scaleFactor > 1.0)
    {
        scaleFactor -= 1.0;
    }
    else if (scaleFactor <= 1.0)
    {
        scaleFactor *= 0.6667;
        if (scaleFactor >= 0.35 && scaleFactor < 0.5)  scaleFactor = 0.4;
        if (scaleFactor < 0.35)  scaleFactor = 0.4;
    }

    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Scaling factor: " + QString::number(scaleFactor));
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    if (grabMode == 'C')  displayFrame();
}

void MainWindow::on_actionZoomToRaw_triggered()
{
    scaleFactor = 1.0;
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Scaling factor: " + QString::number(scaleFactor));
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    if (grabMode == 'C')  displayFrame();
}

void MainWindow::receiveShowMousePosition(QPoint &pos)
{
    ui->labelShowPos->setText(QString::number(pos.x()) + ", " + QString::number((pos.y())) );
}

void MainWindow::receiveFrameRequest()
{
    qDebug() << "Receive Frame Request in MainWindow";
    emit sendFrametoCalibrator(cvRawFrameCopy);
}

void MainWindow::on_actionMCalibrate_triggered()
{
    ui->pushButtonStartCali->setEnabled(true);
    ui->pushButtonRedoCali->setEnabled(false);
    ui->pushButtonConfirm->setEnabled(false);
    ui->pushButtonCalculate->setEnabled(false);
    ui->lineEditRealDistance->setEnabled(false);
    ui->labelRealDisName->setEnabled(false);
    ui->labelCalResult->setEnabled(false);

    manualCalibration = true;
    autoCalibration = false;
    setMCaliVisible(true);

    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Manual calibration selected.");
}

void MainWindow::on_actionOpenImage_triggered()
{
    grabMode = 'C';
    ui->labelShowRes->setText("2448x2048");
    ui->labelShowFrame->setMouseTracking(true);
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");

    QString openFilePath = QFileDialog::getOpenFileName(this, "Open an image", "../");
    QByteArray ba = openFilePath.toLatin1();
    const char *imagePath = ba.data();
    if (openFilePath.isEmpty())  imagePath = "../images/ex.jpg";
    cv::Mat openImage = cv::imread(imagePath, 1);
    receiveRawFrame(openImage);
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Opened an image.");
}

void MainWindow::on_pushButtonStartCali_clicked()
{
    if (manualCalibration)
    {
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

        QDoubleValidator *validator = new QDoubleValidator(0.00, 9999.99, 2);
        ui->lineEditRealDistance->setValidator(validator);
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "     Manual calibration started.");
    }
    if (autoCalibration)
    {
        ui->pushButtonRedoCali->setEnabled(true);
        ui->pushButtonStartCali->setEnabled(false);
        ui->pushButtonCalculate->setEnabled(true);
        ui->lineEditRealDistance->setEnabled(true);
        ui->labelRealDisName->setEnabled(true);
        ui->labelCalResult->setEnabled(true);
        ui->pushButtonConfirm->setEnabled(false);
        ui->lineEditRealDistance->clear();
        ui->labelCalResult->clear();

        QDoubleValidator *validator = new QDoubleValidator(0.00, 9999.99, 2);
        ui->lineEditRealDistance->setValidator(validator);
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    Auto Calibration started.");

        frameToCali = cvRawFrameCopy.clone();

        RulerCalibrator *rulerCalibrator = new RulerCalibrator(frameToCali, pixelDistanceAC);
        delete rulerCalibrator;

        cv::resize(frameToCali, cvRGBFrame, cv::Size(), scaleFactor, scaleFactor);
        cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
        qDisplayedFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
        ui->labelShowFrame->setPixmap(QPixmap::fromImage(qDisplayedFrame));
    }
}

void MainWindow::on_pushButtonRedoCali_clicked()
{
    if (manualCalibration)
    {
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
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    Manual calibration restarted.");
    }
    if (autoCalibration)
    {
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
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    Auto Calibration started.");

        frameToCali = cvRawFrameCopy.clone();
        RulerCalibrator *rulerCalibrator = new RulerCalibrator(frameToCali, pixelDistanceAC);
        delete rulerCalibrator;

        cv::resize(frameToCali, cvRGBFrame, cv::Size(), scaleFactor, scaleFactor);
        cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
        qDisplayedFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
        ui->labelShowFrame->setPixmap(QPixmap::fromImage(qDisplayedFrame));
    }
}

void MainWindow::on_pushButtonCalculate_clicked()
{
    if (manualCalibration)
    {
        if (ui->lineEditRealDistance->text().isEmpty())
        {
            QMessageBox::information(this, "No Input", "Please fill in number only.");
        }
        else
        {
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                              + "    Calculate calibration results.");
            QString  distance = ui->lineEditRealDistance->text();
            double realDistance = distance.toDouble();
            double pixelDistance = ui->labelShowFrame->getAverageDistance();
            pixelPerMM = pixelDistance / realDistance;
            ui->labelCalResult->setText(QString::number(pixelPerMM, 'f', 2) + " pixel/mm");
            ui->pushButtonConfirm->setEnabled(true);
        }
    }
    if (autoCalibration)
    {
        if (ui->lineEditRealDistance->text().isEmpty())
        {
            QMessageBox::information(this, "No Input", "Please fill in number only.");
        }
        else
        {
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                              + "    Calculate calibration results.");
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
    if(!caliConfFile.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
    {
         QMessageBox::warning(this,"File Write Error","Conf file can't open",QMessageBox::Yes);
    }
    else
    {
        QTextStream in(&caliConfFile);
        in << QString::number(pixelPerMM, 'f', 2) << "\n";
        caliConfFile.close();
    }
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Calibration results updated to conf file.");
}

void MainWindow::on_pushButtonConfirm_clicked()
{
    if (manualCalibration || autoCalibration)
    {
        ui->labelShowFrame->finishMCalibration();
        setMCaliVisible(false);
        QMessageBox::StandardButton updateConfReply;
        updateConfReply = QMessageBox::question(this, "Update Configuration",
                                                "Do you want to update configuration file?",
                                                QMessageBox::Yes|QMessageBox::No);
        if (updateConfReply == QMessageBox::Yes)
        {
            writeCaliConf();
            currentPPMM = pixelPerMM;
        }
    }
    manualCalibration = false;
    autoCalibration = false;
    displayFrame();
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
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Auto calibration selected.");
}

void MainWindow::on_actionAutoRulerStart_triggered()
{
    autoMeasure = true;
    frameToMeasure = cvRawFrameCopy.clone();
    MeasureTool *measureTool = new MeasureTool(frameToMeasure, currentPPMM);
    delete measureTool;

    cv::resize(frameToMeasure, cvRGBFrame, cv::Size(), scaleFactor, scaleFactor);
    cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
    qDisplayedFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
    ui->labelShowFrame->setPixmap(QPixmap::fromImage(qDisplayedFrame));
}

void MainWindow::on_actionAutoRulerStop_triggered()
{
    autoMeasure = false;
    displayFrame();
}

void MainWindow::on_actionManualRulerStart_triggered()
{
    ui->labelShowFrame->startManualRuler(currentPPMM);
}

void MainWindow::on_actionManualRulerStop_triggered()
{
    ui->labelShowFrame->stopManualRuler();
}

void MainWindow::on_actionCameraSetting_triggered()
{
    SettingDialog *settingDialog = new SettingDialog(this);
    int r = settingDialog->exec();
    if (r == QDialog::Accepted)
    {
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                          + "    Camera Parameters Updated.");
        if (ui->pushButtonDisconnect->isEnabled())
        {
            if (!ui->pushButtonStream->isEnabled())
            {
                on_pushButtonStop_clicked();
                on_pushButtonDisconnect_clicked();
                on_pushButtonConnect_clicked();
                on_pushButtonStream_clicked();
            }
            else
            {
                on_pushButtonDisconnect_clicked();
                on_pushButtonConnect_clicked();
                on_pushButtonCapture_clicked();
            }
        }
    }
    delete settingDialog;
}

void MainWindow::on_pushButtonMatch_clicked()
{
    matchMethod = ui->comboBoxMatchMethod->currentText();
    frameToTest = cvRawFrameCopy.clone();

    if (matchMethod == "Machine Learning")
    {
        QString matchResult = pyClassification->process(frameToTest);
        ui->labelMatchResult->setStyleSheet("color: blue; font: 20pt; background-color: white;");
        ui->labelMatchResult->setText(matchResult);
    }
    else if (matchMethod == "Image Processing")
    {
        QElapsedTimer timer;
        timer.start();
        QMap<QString, double> testDists = fdTester->getTestDistance(frameToTest);

        QMapIterator<QString, double> i(testDists);
        QString bestMatchName = "None";
        double minDist = 9999.99;

        qDebug() << "Total time:" << timer.elapsed() << "ms";
        qDebug() << "------------------------------------------------";

        while (i.hasNext())
        {
            i.next();
            if (i.value() <= minDist)
            {
                minDist = i.value();
                bestMatchName = i.key();
            }/*
            if (minDist > 0.999)
            {
                bestMatchName = "None";
            }*/
            ui->labelMatchResult->setText(bestMatchName);
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                               + "    " + i.key() + ": " + QString::number(i.value()));
        }
    }
}

void MainWindow::on_actionCalibration_triggered()
{
    CalibratorForm *calibratorForm = new CalibratorForm();
    calibratorForm->show();

    connect(calibratorForm, SIGNAL(sendFrameRequest()), this, SLOT(receiveFrameRequest()));
    connect(this, SIGNAL(sendFrametoCalibrator(cv::Mat)), calibratorForm, SLOT(receiveFrame(cv::Mat)));
}
