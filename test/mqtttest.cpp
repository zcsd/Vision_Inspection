#include "mqtttest.h"
#include "ui_mqtttest.h"

MqttTest::MqttTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MqttTest)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);
}

MqttTest::~MqttTest()
{
    delete ui;
}

void MqttTest::on_pushButtonConnect_clicked()
{

}

void MqttTest::on_pushButtonDisconnect_clicked()
{

}

void MqttTest::on_pushButtonPublish_clicked()
{

}

void MqttTest::on_pushButtonSub_clicked()
{

}

void MqttTest::on_pushButtonUnsub_clicked()
{

}
