#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    frameGrabber = new FrameGrabber();
    measureTool = new MeasureTool();
    pyClassification = new PyClassification();
    fdTester = new FDTester();
    triggerForm = new TriggerForm(this);
    //modbusTest = new ModbusTest(this);
    //mqttTest = new MqttTest(this);
    //opcuaTest = new OpcUaTest(this);
    //rfidTest = new RFIDtest(this);

    initialSetup();
}

MainWindow::~MainWindow()
{
    if (isOpcUaConnected)
    {
        visionStatusNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, 0, QOpcUa::UInt16);
        diconnectToOPCUA();
    }
    delete pyClassification;
    delete fdTester;
    delete frameGrabber;
    delete measureTool;
    //delete calibratorForm;
    delete triggerForm;
    //delete modbusClient;
    delete ui;
}

void MainWindow::initialSetup()
{
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Vision Inspection System started.");
    ui->labelShowFrame->setScaledContents(true);
    ui->scrollArea->setBackgroundRole(QPalette::Dark);
    ui->scrollArea->setWidget(ui->labelShowFrame);
    ui->scrollArea->setVisible(true);

    opcuaProvider = new QOpcUaProvider(this);
    receiveReadCaliConf();

    bgImg.load("../resource/logo.png");
    ui->labelShowFrame->setPixmap(bgImg);

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

    connect(this, SIGNAL(sendFrameToMeasurement(cv::Mat)), measureTool, SLOT(receiveFrame(cv::Mat)));
    connect(this, SIGNAL(sendCalibrationPara(double, int)), measureTool, SLOT(receiveCalibrationPara(double, int)));
    connect(measureTool, SIGNAL(sendFrameToShow(cv::Mat)), this, SLOT(receiveRawFrame(cv::Mat)));
    connect(measureTool, SIGNAL(sendMeasurement(double)), this, SLOT(receiveMeasurement(double)));

    connect(this, SIGNAL(sendStatusToWriteResult()), this, SLOT(receiveStatusToWriteResult()));
    connect(this, SIGNAL(sendResultToCheck()), this, SLOT(receiveResultToCheck()));
    emit sendCalibrationPara(currentPPMM, 3);

    ui->comboBoxMatchMethod->addItems({"Machine Learning", "Image Processing"});
    connectToOPCUA();
}

void MainWindow::receiveReadCaliConf()
{
    QFile caliConfFile("../conf/calibration.conf");

    if (!caliConfFile.open(QIODevice::ReadOnly))
    {
         QMessageBox::warning(this,"File Read Error", "Configuration file can't open", QMessageBox::Yes);
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

void MainWindow::receiveMeasurement(double length)
{
    ui->labelShowMeasurement->setText(QString::number(length, 'f', 2) + " mm");
    // for PoC
    currentLength = length;
    emit sendResultToCheck();
}

void MainWindow::on_pushButtonConnect_clicked()
{
    emit sendConnect();
    usleep(1000); // 1ms

    if (frameGrabber->cameraConnected)
    {
        //visionStatusNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, 1, QOpcUa::UInt16);
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
        usleep(1000);

        if (!frameGrabber->cameraConnected)
        {
            //visionStatusNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, 0, QOpcUa::UInt16);
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
            usleep(1000);

            if (!frameGrabber->cameraConnected)
            {
                on_pushButtonStop_clicked();
                //visionStatusNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, 0, QOpcUa::UInt16);
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
        usleep(1000);

        if (!frameGrabber->cameraConnected)
        {
            //visionStatusNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, 0, QOpcUa::UInt16);
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

    ui->labelShowRes->setText("2448x2048"); // hard code need to be changed
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
                                          + "    One image saved.");
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
    ui->labelShowRes->setText("2448x2048"); // hard code, need to be changed
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Start streaming mode.");
}

void MainWindow::on_pushButtonStop_clicked()
{
    if (grabMode == 'S')
    {
        streamTrigger->stop();
        delete streamTrigger;

        ui->labelShowFrame->setMouseTracking(false);
        emit sendStopGrabbing();
        usleep(1000);

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

void MainWindow::connectToOPCUA()
{
    const static QUrl opcuaServer(QLatin1String("opc.tcp://172.19.80.34:4840"));
    // default plugin is open62541
    //qDebug() << "Available OPCUA plugins:" << opcuaProvider->availableBackends();
    opcuaClient = opcuaProvider->createClient(opcuaProvider->availableBackends()[0]);

    if (!opcuaClient)
    {
        qDebug() << "Fail to create OPCUA client.";
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                                  + "Fail to create OPCUA client.");
        return;
    }

    connect(opcuaClient, &QOpcUaClient::connected, this, &MainWindow::opcuaConnected);
    connect(opcuaClient, &QOpcUaClient::disconnected, this, &MainWindow::opcuaDisconnected);
    connect(opcuaClient, &QOpcUaClient::errorChanged, this, &MainWindow::opcuaClientError);
    connect(opcuaClient, &QOpcUaClient::stateChanged, this, &MainWindow::opcuaClientState);

    opcuaClient->connectToEndpoint(opcuaServer); // connect action
}

void MainWindow::diconnectToOPCUA()
{
    if (isOpcUaConnected)
    {
        opcuaClient->disconnectFromEndpoint();
    }
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
    emit sendFrameToCalibrator(cvRawFrameCopy);
}

void MainWindow::on_actionOpenImage_triggered()
{
    grabMode = 'C';

    QString openFilePath = QFileDialog::getOpenFileName(this, "Open an image", "../images/");
    QByteArray ba = openFilePath.toLatin1();
    const char *imagePath = ba.data();
    if (openFilePath.isEmpty())  imagePath = "../resource/logo.png";
    cv::Mat openImage = cv::imread(imagePath, 1);
    receiveRawFrame(openImage);
    ui->labelShowRes->setText("2448x2048"); // hard code
    ui->labelShowFrame->setMouseTracking(true);
    ui->labelShowScale->setText(QString::number(scaleFactor*100, 'f', 0)+"%");
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                      + "    Opened an image.");
}

void MainWindow::on_actionAutoRulerStart_triggered()
{
    autoMeasure = true;
    frameToMeasure = cvRawFrameCopy.clone();

    /*
    MeasureTool *measureTool = new MeasureTool(frameToMeasure, currentPPMM);
    delete measureTool;

    cv::resize(frameToMeasure, cvRGBFrame, cv::Size(), scaleFactor, scaleFactor);
    cv::cvtColor(cvRGBFrame, cvRGBFrame, cv::COLOR_BGR2RGB);
    qDisplayedFrame = QImage((uchar*)cvRGBFrame.data, cvRGBFrame.cols, cvRGBFrame.rows, cvRGBFrame.step, QImage::Format_RGB888);
    ui->labelShowFrame->setPixmap(QPixmap::fromImage(qDisplayedFrame));
    */
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

        // for PoC
        colorStatus = 8;
        emit sendResultToCheck();
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
            ui->labelMatchResult->setStyleSheet("color: blue; font: 20pt; background-color: white;");
            ui->labelMatchResult->setText(bestMatchName);  
            ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                               + "    " + i.key() + ": " + QString::number(i.value()));
        }
    }
}

void MainWindow::on_actionCalibration_triggered()
{
    calibratorForm = new CalibratorForm(this);
    calibratorForm->show();

    connect(calibratorForm, SIGNAL(sendFrameRequest()), this, SLOT(receiveFrameRequest()));
    connect(this, SIGNAL(sendFrameToCalibrator(cv::Mat)), calibratorForm, SLOT(receiveFrame(cv::Mat)));
    connect(calibratorForm, SIGNAL(sendFrameToShow(cv::Mat)), this, SLOT(receiveRawFrame(cv::Mat)));
    connect(calibratorForm, SIGNAL(sendUpdateConfig()), this, SLOT(receiveReadCaliConf()));
    connect(calibratorForm, SIGNAL(sendCaliCommand(QString)), this->ui->labelShowFrame, SLOT(receiveCaliCommand(QString)));
    connect(this->ui->labelShowFrame, SIGNAL(sendMousePressedPosition(QPoint&)), calibratorForm, SLOT(receiveMousePressedPosition(QPoint&)));
}

void MainWindow::on_pushButtonMeasure_clicked()
{
    emit sendFrameToMeasurement(cvRawFrameCopy);
}

void MainWindow::on_actionTrigger_triggered()
{
    triggerForm->show();
    connect(triggerForm, SIGNAL(sendTrigger()), this, SLOT(receiveTrigger()));
}

void MainWindow::receiveTrigger()
{
    objectPresentNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, 1, QOpcUa::UInt16);
    qDebug() << "Part Present by trigger.";
    // NOT GOOD, main thread will stop
    usleep(2000000); // 2s

    on_pushButtonCapture_clicked();
    on_pushButtonMeasure_clicked();
}

void MainWindow::on_actionModbus_triggered()
{
    //modbusTest->show();
}

void MainWindow::on_actionMQTT_triggered()
{
    //mqttTest->show();
}

void MainWindow::on_actionRFID_triggered()
{
    //rfidTest->show();
}

void MainWindow::on_actionOPC_UA_triggered()
{
    //opcuaTest->show();
}

void MainWindow::receiveResultToCheck()
{
    colorStatus = 8; // remember to delete
    if (currentLength != 0.0 && colorStatus != 0)
    {
        if (colorStatus != 8)
        {
            visionResult = 10;
            isResultReady = true;
            emit sendStatusToWriteResult();
        }
        else if (colorStatus == 8)
        {
            if (abs(currentLength-standardLengh) < 1.0)
            {
                visionResult = 8;
                isResultReady = true;
                emit sendStatusToWriteResult();
            }
            else
            {
                visionResult = 9;
                isResultReady = true;
                emit sendStatusToWriteResult();
            }
        }
        currentLength = 0.0;
        colorStatus = 0;
    }
}

void MainWindow::receiveStatusToWriteResult()
{
    if (isMachineReady && isResultReady)
    {
        visionResultNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, visionResult, QOpcUa::UInt16);
        isResultReady = false;
    }
}

void MainWindow::opcuaConnected()
{
    visionStatusNodeW = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.vision.VISION_STATUS"); // uint 16
    // temp using
    visionStatusNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, 1, QOpcUa::UInt16);
    visionStatusNodeW->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
    connect(visionStatusNodeW, &QOpcUaNode::attributeUpdated, this, [this](QOpcUa::NodeAttribute attr, const QVariant &value)
    {
        Q_UNUSED(attr);
        if (value.toInt() != 1)
        {
            visionStatusNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, 1, QOpcUa::UInt16);
        }
    });

    visionResultNodeW = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.vision.RESULT"); // uint 16
    objectPresentNodeW = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.objectPresent"); // uint 16

    resultReadNodeRW = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.vision.RESULT_READ"); // uint 16
    resultReadNodeRW->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
    connect(resultReadNodeRW, &QOpcUaNode::attributeUpdated, this, [this](QOpcUa::NodeAttribute attr, const QVariant &value)
    {
        Q_UNUSED(attr);
        qDebug() << "Read result-read status node:" << value.toInt();
        if (value.toInt() == 1)
        {
            visionResultNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, 0, QOpcUa::UInt16);
            resultReadNodeRW->writeAttribute(QOpcUa::NodeAttribute::Value, 0, QOpcUa::UInt16);
        }
    });

    machinePLCReadyNodeRW = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.vision.MACHINE_READY"); // unit 16
    machinePLCReadyNodeRW->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
    connect(machinePLCReadyNodeRW, &QOpcUaNode::attributeUpdated, this, [this](QOpcUa::NodeAttribute attr, const QVariant &value)
    {
        Q_UNUSED(attr);
        qDebug() << "Read PLC-Machine-Ready status node:" << value.toInt();
        if (value.toInt() == 0)
        {
            isMachineReady = false;
        }
        else if (value.toInt() == 1)
        {
            isMachineReady = true;
            emit sendStatusToWriteResult();
        }

    });

    jobModelNodeR = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.job.job_Model"); // string
    jobModelNodeR->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
    connect(jobModelNodeR, &QOpcUaNode::attributeUpdated, this, [this](QOpcUa::NodeAttribute attr, const QVariant &value)
    {
        Q_UNUSED(attr);
        qDebug() << "Read job-model infor node:" << value.toString();
        standardModel = value.toString();
    });

    jobLengthNodeR = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.job.job_Length"); // int 16
    jobLengthNodeR->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
    connect(jobLengthNodeR, &QOpcUaNode::attributeUpdated, this, [this](QOpcUa::NodeAttribute attr, const QVariant &value)
    {
        Q_UNUSED(attr);
        qDebug() << "Read job-length node:" << value.toInt();
        standardLengh = double(value.toInt());
    });

    jobColorNodeR = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.job.job_Color"); // string
    jobColorNodeR->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
    connect(jobColorNodeR, &QOpcUaNode::attributeUpdated, this, [this](QOpcUa::NodeAttribute attr, const QVariant &value)
    {
        Q_UNUSED(attr);
        qDebug() << "Read job-color status node:" << value.toString();
        standardColor = value.toString();
    });
}

void MainWindow::opcuaDisconnected()
{
    isOpcUaConnected = false;
    opcuaClient->deleteLater();
}

void MainWindow::opcuaClientError(QOpcUaClient::ClientError error)
{
    qDebug() << "OPCUA Client Error:" << error;
    ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                              + "OPCUA Client Error: " + error);
}

void MainWindow::opcuaClientState(QOpcUaClient::ClientState state)
{
    if (state == QOpcUaClient::ClientState::Connected)
    {
        qDebug() << "Connected to OPCUA server.";
        isOpcUaConnected = true;
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                                  + "Connected to OPCUA server.");
    }
    else if (state == QOpcUaClient::ClientState::Connecting)
    {
        qDebug() << "Trying to connect OPCUA server now.";
        isOpcUaConnected = false;
    }
    else if (state == QOpcUaClient::ClientState::Disconnected)
    {
        qDebug() << "Disconnected to OPCUA server.";
        isOpcUaConnected = false;
        ui->listWidgetMessageLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                                  + "Disconnected to OPCUA server.");
    }
}

void MainWindow::on_pushButtonVisionResultReady_clicked()
{
    visionResult = ui->lineEditVisionResult->text().toInt();
    // temp using
    colorStatus = visionResult;
    isResultReady = true;
    emit sendStatusToWriteResult();
    qDebug() << "visionResult:" << visionResult;
}

void MainWindow::on_pushButtonPartPresent_clicked()
{
    objectPresentNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, 1, QOpcUa::UInt16);
    qDebug() << "Part Present Manually.";
}
