#include "modbusclient.h"

ModbusClient::ModbusClient(QObject *parent, QString _ip, QString _port, bool _keepAlive) :
    QObject(parent)
{
    ip = QVariant(_ip);
    port = QVariant(_port);
    keepAlive = _keepAlive;
    initSetup();
}

ModbusClient::~ModbusClient()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;
}

void ModbusClient::initSetup()
{
    if (modbusDevice)
    {
        modbusDevice = nullptr;
    }
    modbusDevice = new QModbusTcpClient(this);
    // immediate response to connection status change
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
        if (!modbusDevice->connectDevice())
        {
            qDebug() << "Modbus connection failure.";
        }
    }
}

void ModbusClient::disconnectToPLC()
{
    if (!modbusDevice)
        return;

    if (modbusDevice->state() == QModbusDevice::ConnectedState)
    {
        modbusDevice->disconnectDevice();
    }
}

// Client will lose connection if no any activity in 30s
void ModbusClient::onStateChanged(int state)
{
    if (state == QModbusDevice::UnconnectedState)
    {
        connectionStatus = false;
        if (keepAlive)
        {
            // auto connect to PLC if connection lose
            connectToPLC();
        }
    }
    else if (state == QModbusDevice::ConnectedState)
    {
        connectionStatus = true;
    }
}

bool ModbusClient::connected()
{
    return connectionStatus;
}

void ModbusClient::writeToPLC(int regAddress, int msgToSend)
{
    if (!modbusDevice)
        return;

    if (modbusDevice->state() != QModbusDevice::ConnectedState)
    {
        connectToPLC();
    }
    // datatype, start register address, how many resgister to write
    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, regAddress, 1);

    for (uint i = 0; i < writeUnit.valueCount(); i++) {
        writeUnit.setValue(int(i), msgToSend); // write integer directly
    }

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, 1))
    {
        if (!reply->isFinished())
        {
            //reply Returns true when the reply has finished or was aborted.
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError)
                {
                    qDebug() << "Write response error:" << reply->errorString() << reply->rawResult().exceptionCode();
                }
                else if (reply->error() != QModbusDevice::NoError)
                {
                    qDebug() << "Write response error:" << reply->errorString() << reply->error();
                }

                reply->deleteLater();
            });
        }
        else
        {
            reply->deleteLater(); // broadcast replies return immediately
        }
    }
    else
    {
        qDebug() << "Write error:" << modbusDevice->errorString();
    }
}

void ModbusClient::readFromPLC(int regAddress)
{
    if (!modbusDevice)
        return;

    if (modbusDevice->state() != QModbusDevice::ConnectedState)
    {
        connectToPLC();
    }

    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, regAddress, 1);
    if (auto *reply = modbusDevice->sendReadRequest(readUnit, 1))
    {
        if (!reply->isFinished())
        {
            connect(reply, &QModbusReply::finished, this, &ModbusClient::readReady);
        }
        else
        {
            delete reply; // broadcast replies return immediately
        }
    }
    else
    {
        qDebug() << "Read Erroor:" << modbusDevice->errorString();
    }
}

void ModbusClient::readReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError)
    {
        // msg store in reply.reslut()
        const QModbusDataUnit unit = reply->result();
        for (uint i = 0; i < unit.valueCount(); i++)
        {
            const QString startAdd = tr("%1").arg(unit.startAddress());
            bool ok;
            // convert from hex to decimal
            const unsigned int parsedValue = QString::number(unit.value(i),
                                                             unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16).toUInt(&ok, 16);
            qDebug() << "Read from" << startAdd << ":" << QString::number(parsedValue);
            emit sendReadMsg(ip.toString(), startAdd.toInt(), int(parsedValue));
        }
    }
    else if (reply->error() == QModbusDevice::ProtocolError)
    {
        qDebug() << "Read response error:" << reply->errorString() << reply->rawResult().exceptionCode();
    }
    else
    {
        qDebug() << "Read response error:" << reply->errorString() << reply->error();
    }

    reply->deleteLater();
}
