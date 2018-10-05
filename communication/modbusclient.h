// ModbusClient class: communicate with Modbus PLC
// Author: @ZC
// Date: created on 04 Oct 2018
#ifndef MODBUSCLIENT_H
#define MODBUSCLIENT_H

#include <QObject>
#include <QWidget>
#include <QModbusClient>
#include <QModbusTcpClient>
#include <QModbusDataUnit>
#include <QDebug>

namespace Ui {
class ModbusClient;
}

class ModbusClient : public QWidget
{
    Q_OBJECT

public:
    explicit ModbusClient(QWidget *parent = nullptr, QString _ip = "172.19.80.32", QString _port = "502");
    ~ModbusClient();
    bool connected();
    void writeToPLC(int regAddress, int msgToSend);
    void readFromPLC(int regAddress);

signals:
    void sendReadMsg(int regAddress, int msgRead);

public slots:

private slots:
    void onStateChanged(int state);
    void readReady();

private:
    Ui::ModbusClient *ui;
    QModbusClient *modbusDevice;
    QVariant ip, port;
    bool connectionStatus = false;
    void initSetup();
    void connectToPLC();
    void disconnectToPLC();
};

#endif // MODBUSCLIENT_H
