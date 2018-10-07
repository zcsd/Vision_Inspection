#include "modbustest.h"
#include "ui_modbustest.h"

ModbusTest::ModbusTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModbusTest)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);

    initSetup();
}

ModbusTest::~ModbusTest()
{
    delete ui;
}

void ModbusTest::initSetup()
{
    ui->lineEditIP->setText("172.19.80.32");
    ui->lineEditPort->setText("502");
    ui->lineEditReadAddress->setText("38268");
    ui->lineEditSendAddress->setText("37768");

    receiveConnectionStatus(false);
}

void ModbusTest::on_pushButtonConnect_clicked()
{
    ip = ui->lineEditIP->text();
    port = ui->lineEditPort->text();
    modbusClient = new ModbusClient(this, ip, port, keepAlive);
    connect(modbusClient, SIGNAL(sendReadMsg(QString, int, int)), this, SLOT(receiveReadMsg(QString, int, int)));
    connect(modbusClient, SIGNAL(sendConnectionStatus(bool)), this, SLOT(receiveConnectionStatus(bool)));
    modbusClient->connectToPLC();
}

void ModbusTest::on_pushButtonDisconnect_clicked()
{
    modbusClient->disconnectToPLC();
}

void ModbusTest::receiveReadMsg(QString ip, int regAddress, int msgRead)
{
    ui->labelShowRead->setText(QString::number(msgRead));
}

void ModbusTest::receiveConnectionStatus(bool connected)
{
    if (connected)
    {
        isConnected = true;
        ui->pushButtonConnect->setDisabled(true);
        ui->pushButtonDisconnect->setEnabled(true);
        ui->pushButtonConnect->setStyleSheet("background-color: rgb(100, 255, 100);");
        ui->pushButtonRead->setEnabled(true);
        ui->pushButtonSend->setEnabled(true);
    }
    else
    {
        isConnected = false;
        ui->pushButtonConnect->setEnabled(true);
        ui->pushButtonDisconnect->setDisabled(true);
        ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);");
        ui->pushButtonRead->setDisabled(true);
        ui->pushButtonSend->setDisabled(true);
    }
}

void ModbusTest::on_pushButtonSend_clicked()
{
    if(!ui->lineEditSend->text().isEmpty() && !ui->lineEditSend->text().isEmpty())
    {
        modbusClient->writeToPLC(ui->lineEditSendAddress->text().toInt(), ui->lineEditSend->text().toInt());
    }
    else
    {
       QMessageBox::warning(this, "No enough information", "Please fill in address and content to send.");
    }

}

void ModbusTest::on_pushButtonClearSend_clicked()
{
    ui->lineEditSend->clear();
}

void ModbusTest::on_pushButtonRead_clicked()
{
    if(!ui->lineEditReadAddress->text().isEmpty())
    {
        modbusClient->readFromPLC(ui->lineEditReadAddress->text().toInt());
    }
    else
    {
        QMessageBox::warning(this, "No enough information", "Please fill in address.");
    }

}

void ModbusTest::on_pushButtonClearRead_clicked()
{
    ui->labelShowRead->clear();
}
