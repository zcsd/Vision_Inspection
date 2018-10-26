// RFIDTool class: Read RFID tag
// Author: @ZC
// Date: created on 12 Oct 2018
#ifndef RFIDTOOL_H
#define RFIDTOOL_H

#include <QObject>
#include <QDebug>

#include <stdio.h>
#include <string.h>

#include <libs8.h> // RFID lib

class RFIDTool : public QObject
{
    Q_OBJECT
public:
    explicit RFIDTool(QObject *parent = nullptr);
    bool initDevice();
    void closeDevice();
    bool testDevice();


signals:
    void sendDeviceInfo(QString);
    void sendReadInfo(bool, QString, QString);

public slots:
    void icode2();

private:
    int icdev;

};

#endif // RFIDTOOL_H
