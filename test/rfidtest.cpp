#include "rfidtest.h"
#include "ui_rfidtest.h"

RFIDtest::RFIDtest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RFIDtest)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);

    rfidTool = new RFIDTool(this);
    connect(rfidTool, SIGNAL(sendDeviceInfo(QString)), this, SLOT(receiveDeviceInfo(QString)));
    connect(rfidTool, SIGNAL(sendReadInfo(bool, QString, QString)), this, SLOT(receiveReadInfo(bool, QString, QString)));
    ui->labelShowPort->clear();
    connect(ui->listWidget->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            ui->listWidget, SLOT(scrollToBottom()));
}

RFIDtest::~RFIDtest()
{
    delete readThread;
    delete readTrigger;
    delete ui;
}

void RFIDtest::on_pushButtonConnect_clicked()
{
    if (rfidTool->initDevice())
    {
        ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Connect to RFID Scanner.");
        readThread = new QThread(this);
        readThread->start();

        readTrigger = new QTimer();
        readTrigger->setInterval(500);

        connect(readTrigger, SIGNAL(timeout()), rfidTool, SLOT(icode2()), Qt::DirectConnection);
        connect(readThread, SIGNAL(finished()), readTrigger, SLOT(stop()));

        readTrigger->start();
        //run timer work(loop reading RFID tag) in thread
        readTrigger->moveToThread(readThread);
    }
}

void RFIDtest::on_pushButtonClose_clicked()
{
    //readTrigger->stop();
    readThread->quit();
    readThread->wait();

    rfidTool->closeDevice();
    ui->labelShowPort->clear();
    ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                      + "Disconnect to RFID Scanner.");
}

void RFIDtest::on_pushButtonRead_clicked()
{
    rfidTool->icode2();
}

void RFIDtest::receiveDeviceInfo(QString port)
{
    ui->labelShowPort->setText(port);
}

void RFIDtest::receiveReadInfo(bool isValid, QString card, QString data)
{
    //qDebug() << isValid << card << data;
    if (isValid)
    {
        ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + card + ": " +  data.split(":")[2]);
    }
    else
    {
        ui->listWidget->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + card);
    }
}
