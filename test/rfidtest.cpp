#include "rfidtest.h"
#include "ui_rfidtest.h"

RFIDtest::RFIDtest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RFIDtest)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);

    rfidTool = new RFIDTool(this);
}

RFIDtest::~RFIDtest()
{
    delete ui;
}

void RFIDtest::on_pushButtonOpen_clicked()
{
    rfidTool->initDevice();
}

void RFIDtest::on_pushButtonClose_clicked()
{

}

void RFIDtest::on_pushButtonRead_clicked()
{
    rfidTool->icode2();
}
