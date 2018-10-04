#include "modbusclient.h"
#include "ui_modbusclient.h"

ModbusClient::ModbusClient(QWidget *parent, QString _ip, QString _port) :
    QWidget(parent),
    ui(new Ui::ModbusClient)
{
    ui->setupUi(this);

    ip = QVariant(_ip);
    port = QVariant(_port);
    initSetup();
}

ModbusClient::~ModbusClient()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;
    delete ui;
}

void ModbusClient::initSetup()
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = nullptr;
    }
    modbusDevice = new QModbusTcpClient(this);
    connect(modbusDevice, &QModbusClient::stateChanged, this, &ModbusClient::onStateChanged);
}

void ModbusClient::connectToPLC()
{
    if (!modbusDevice)
        return;

    if (modbusDevice->state() != QModbusDevice::ConnectedState)
    {
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, ip);
        modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);

        modbusDevice->setTimeout(1000); // 1s
        modbusDevice->setNumberOfRetries(3);
        if (!modbusDevice->connectDevice()) {
            qDebug() << "Modbus connection failure.";
        }
    }

}

void ModbusClient::onStateChanged(int state)
{
    if (state == QModbusDevice::UnconnectedState)
    {
        connectionStatus = false;
        connectToPLC();
    }
    else if (state == QModbusDevice::ConnectedState)
    {
        connectionStatus = true;
        qDebug() << "11";
    }
}

bool ModbusClient::connected()
{
    return connectionStatus;
}

bool ModbusClient::writeToPLC(int regAddress, int msgToSend)
{

}

int ModbusClient::readFromPLC(int regAddress)
{

}
