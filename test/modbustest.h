// ModbusTest class: test Modbus connection and function
// Author: @ZC
// Date: created on 05 Oct 2018
#ifndef MODBUSTEST_H
#define MODBUSTEST_H

#include <QWidget>
#include <QtDebug>
#include <QObject>
#include <QMessageBox>

#include <communication/modbusclient.h>

namespace Ui {
class ModbusTest;
}

class ModbusTest : public QWidget
{
    Q_OBJECT

public:
    explicit ModbusTest(QWidget *parent = nullptr);
    ~ModbusTest();

private slots:
    void receiveReadMsg(QString ip, int regAddress, int msgRead);
    void receiveConnectionStatus(bool connected);

    void on_pushButtonConnect_clicked();
    void on_pushButtonDisconnect_clicked();
    void on_pushButtonSend_clicked();
    void on_pushButtonClearSend_clicked();
    void on_pushButtonRead_clicked();
    void on_pushButtonClearRead_clicked();

private:
    Ui::ModbusTest *ui;
    ModbusClient *modbusClient;
    void initSetup();
    QString ip, port;
    int writeAdress, readAddress;
    bool keepAlive = false;
    bool isConnected = false;
};

#endif // MODBUSTEST_H
