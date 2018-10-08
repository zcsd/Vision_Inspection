// ModbusClient class: communicate with Modbus PLC
// Author: @ZC
// Date: created on 04 Oct 2018
#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <QObject>
#include <QModbusClient>
#include <QModbusTcpClient>
#include <QModbusDataUnit>
#include <QDebug>

class ModbusClient : public QObject
{
    Q_OBJECT
public:
    explicit ModbusClient(QObject *parent = nullptr,
                          QString _ip = "172.19.80.32", QString _port = "502", bool _keepAlive = false);
    ~ModbusClient();

    bool connected();
    void connectToPLC();
    void disconnectToPLC();
    void writeToPLC(int regAddress, int msgToSend);
    void readFromPLC(int regAddress);

signals:
    void sendReadMsg(QString ip, int regAddress, int msgRead);
    void sendConnectionStatus(bool connected);

public slots:

private slots:
    void onStateChanged(int state);
    void readReady();

private:
    QModbusClient *modbusDevice;
    QVariant ip, port;
    bool connectionStatus = false;
    bool keepAlive = false;
    void initSetup();
};

#endif // MODBUSCLIENT_H
