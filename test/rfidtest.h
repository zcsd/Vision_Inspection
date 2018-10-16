// RFIDTest class: test RFID
// Author: @ZC
// Date: created on 12 Oct 2018
#ifndef RFIDTEST_H
#define RFIDTEST_H

#include <QWidget>
#include <QObject>
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
    void on_pushButtonOpen_clicked();

    void on_pushButtonClose_clicked();

    void on_pushButtonRead_clicked();

private:
    Ui::RFIDtest *ui;
    RFIDTool *rfidTool;
};

#endif // RFIDTEST_H
