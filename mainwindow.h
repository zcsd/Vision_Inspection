// MainWindow class
// Author: @ZC
// Date: created on 18 Jun 2018
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QString>
#include <QImage>
#include <QTimer>
#include <QTime>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>
#include <QOpcUaClient>
#include <QOpcUaNode>
#include <QtOpcUa>

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
using namespace cv;

#include "framegrabber.h"
#include "measuretool.h"
#include "fdtester.h"
#include "pyclassification.h"
#include "settingdialog.h"
#include "calibratorform.h"
#include "triggerform.h"
#include "communication/modbusclient.h"
#include "communication/mqttclient.h"
#include "communication/rfidtool.h"
#include "test/modbustest.h"
#include "test/mqtttest.h"
#include "test/rfidtest.h"
#include "test/opcuatest.h"

#include "pocworker.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void sendConnect();
    void sendDisconnect();
    void sendCaptureMode();
    void sendStreamMode();
    void sendStopGrabbing();
    void sendFrameToCalibrator(cv::Mat cvRawFrame);
    void sendFrameToMeasurement(cv::Mat cvRawFrame);
    void sendCalibrationPara(double pPmm, int test1);
    void sendStatusToWriteResult();
    void sendResultToCheck();

private slots:
    void on_pushButtonConnect_clicked();
    void on_pushButtonDisconnect_clicked();
    void on_pushButtonCapture_clicked();
    void on_pushButtonStream_clicked();
    void on_pushButtonStop_clicked();
    void on_pushButtonScanDevices_clicked();
    void on_pushButtonSaveCapture_clicked();
    void on_actionChangeSavePath_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomToFit_triggered();
    void on_actionZoomOut_triggered();
    void on_actionZoomToRaw_triggered();
    void on_actionOpenImage_triggered();
    void on_actionAutoRulerStart_triggered();
    void on_actionAutoRulerStop_triggered();
    void on_actionManualRulerStart_triggered();
    void on_actionManualRulerStop_triggered();
    void on_actionCameraSetting_triggered();
    void on_pushButtonMatch_clicked();
    void on_actionCalibration_triggered();
    void receiveRawFrame(cv::Mat cvRawFrame);
    void receiveReadCaliConf();
    void receiveMeasurement(double length, bool color);
    void on_pushButtonMeasure_clicked();
    void on_actionTrigger_triggered();
    void receiveTrigger();
    void on_actionModbus_triggered();
    void on_actionMQTT_triggered();
    void on_actionRFID_triggered();
    void on_actionOPC_UA_triggered();

    void receiveResultToCheck();
    void receiveStatusToWriteResult();
    void opcuaConnected();
    void opcuaDisconnected();
    void opcuaClientError(QOpcUaClient::ClientError error);
    void opcuaClientState(QOpcUaClient::ClientState state);

    void on_pushButtonVisionResultReady_clicked();
    void on_pushButtonPartPresent_clicked();

public slots:
    void receiveShowMousePosition(QPoint& pos);
    void receiveFrameRequest();

private:
    Ui::MainWindow *ui;
    FrameGrabber *frameGrabber;
    MeasureTool *measureTool;
    PyClassification *pyClassification;
    FDTester *fdTester;
    CalibratorForm *calibratorForm;
    TriggerForm *triggerForm;
    //ModbusTest *modbusTest;
    //MqttTest *mqttTest;
    //OpcUaTest *opcuaTest;
    //RFIDtest *rfidTest;
    QPixmap bgImg;
    QString defaultSavePath = "../images";
    QString matchMethod = "Machine Learning";
    char grabMode = 'N';
    bool manualCalibration = false;
    bool autoCalibration = false;
    bool autoMeasure = false;
    double scaleFactor = 0.4; // 0.4 is fit windows size
    double currentPPMM;
    cv::Mat cvRawFrameCopy;
    cv::Mat frameToCali;
    cv::Mat frameToMeasure;
    cv::Mat cvRGBFrame;
    cv::Mat frameToTest;
    QImage qDisplayedFrame;
    QTimer *streamTrigger;
    void initialSetup();
    void displayFrame();

    QOpcUaProvider *opcuaProvider;
    QOpcUaClient *opcuaClient;
    QOpcUaNode *visionStatusNodeW, *visionResultNodeW, *machinePLCReadyNodeRW, *resultReadNodeRW, *objectPresentNodeW;
    QOpcUaNode *jobModelNodeR, *jobLengthNodeR, *jobColorNodeR;
    bool isOpcUaConnected = false, isResultReady = false, isMachineReady = false;
    int visionResult = 0;
    void connectToOPCUA();
    void diconnectToOPCUA();

    QString standardModel;
    QString standardColor;
    double standardLengh = 0.0;
    double currentLength = 0.0;
    int colorStatus = 0;
    bool isColorOK = true;
};

#endif // MAINWINDOW_H
