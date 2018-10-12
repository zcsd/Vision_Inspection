#ifndef RFIDTOOL_H
#define RFIDTOOL_H

#include <QObject>

#include <stdio.h>
#include <string.h>

#include <libs8.h> // RFID lib

class RFIDTool : public QObject
{
    Q_OBJECT
public:
    explicit RFIDTool(QObject *parent = nullptr);
    int initDevice();
    int testDevice(int icdev);
    int icode2(int icdev);

signals:

public slots:


};

#endif // RFIDTOOL_H
