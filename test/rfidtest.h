// RFIDTest class: test RFID
// Author: @ZC
// Date: created on 12 Oct 2018
#ifndef RFIDTEST_H
#define RFIDTEST_H

#include <QWidget>
#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QThread>

#include <communication/rfidtool.h>

namespace Ui {
class RFIDtest;
}

class RFIDtest : public QWidget
{
    Q_OBJECT

public:
    explicit RFIDtest(QWidget *parent = nullptr);
    ~RFIDtest();

private slots:
    void on_pushButtonConnect_clicked();
    void on_pushButtonClose_clicked();
    void on_pushButtonRead_clicked();

    void receiveDeviceInfo(QString port);
    void receiveReadInfo(bool isValid, QString card, QString data);

private:
    Ui::RFIDtest *ui;
    RFIDTool *rfidTool;
    QTimer *readTrigger;
    QThread *readThread;
};

#endif // RFIDTEST_H
