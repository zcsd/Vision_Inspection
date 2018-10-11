#include "mqtttest.h"
#include "ui_mqtttest.h"

MqttTest::MqttTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MqttTest)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);

    initSetup();
}

MqttTest::~MqttTest()
{
    delete ui;
    if (mqttClient)
        delete mqttClient;
}

void MqttTest::initSetup()
{
    receiveConState(0);
    receiveSubState(0);
    connect(ui->listWidgetStatus->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            ui->listWidgetStatus, SLOT(scrollToBottom()));
    connect(ui->listWidgetMsgSub->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            ui->listWidgetMsgSub, SLOT(scrollToBottom()));
}

void MqttTest::on_pushButtonConnect_clicked()
{
    mqttClient = new MqttClient(this);
    connect(mqttClient, SIGNAL(sendConState(int)), this, SLOT(receiveConState(int)));

    if (mqttClient->isConnected())
    {
        isConnected = true;
    }
    else
    {
        mqttClient->connectToBroker(ui->lineEditIP->text(), ui->lineEditPort->text(),
                                    ui->lineEditUsername->text(), ui->lineEditPassword->text(),
                                    ui->lineEditDeviceID->text(), "8080");
        mqttClient->keepAlive(25);
    }
}

void MqttTest::on_pushButtonDisconnect_clicked()
{
    if (isConnected)
    {
        if (isSub)
        {
            on_pushButtonUnsub_clicked();
        }
        mqttClient->disconnect();
    }
    delete mqttClient;
}

void MqttTest::on_pushButtonPublish_clicked()
{
    if (isConnected)
    {
        mqttClient->publish(ui->lineEditTopicPub->text(), ui->lineEditMsgPub->text(),
                            ui->comboBoxQoSPub->itemData(ui->comboBoxQoSPub->currentIndex()).toInt());
        ui->listWidgetStatus->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Published to " + ui->lineEditTopicPub->text() + ": " + ui->lineEditMsgPub->text());
    }
}

void MqttTest::on_pushButtonSub_clicked()
{
    if (isConnected && !isSub)
    {
        mqttClient->subscribe(ui->lineEditTopicSub->text(),
                              ui->comboBoxQoSSub->itemData(ui->comboBoxQoSSub->currentIndex()).toInt());
        connect(mqttClient, SIGNAL(sendSubMsg(QString, QString)), this, SLOT(receiveSubMsg(QString, QString)));
        connect(mqttClient, SIGNAL(sendSubState(int)), this, SLOT(receiveSubState(int)));
    }
}

void MqttTest::on_pushButtonUnsub_clicked()
{
    if (isConnected && isSub)
    {
        mqttClient->unsubscribe(ui->lineEditTopicSub->text());
        ui->listWidgetStatus->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Unsubscribed to topic " + ui->lineEditTopicSub->text());
    }
}

void MqttTest::receiveConState(int state)
{
    if (state == 1)
    {
        isConnected = true;
        ui->pushButtonConnect->setDisabled(true);
        ui->pushButtonDisconnect->setEnabled(true);
        ui->pushButtonPublish->setEnabled(true);
        ui->pushButtonConnect->setStyleSheet("background-color: rgb(100, 255, 100);");
        if (!isSub)
        {
            ui->pushButtonSub->setEnabled(true);
            ui->pushButtonUnsub->setDisabled(true);
            ui->pushButtonSub->setStyleSheet("background-color: rgb(225, 225, 225);");
        }
        else
        {
            ui->pushButtonSub->setDisabled(true);
            ui->pushButtonUnsub->setEnabled(true);
            ui->pushButtonSub->setStyleSheet("background-color: rgb(100, 255, 100);");
        }
        ui->listWidgetStatus->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Mqtt broker is connected.");
    }
    else
    {
        isConnected = false;
        ui->pushButtonConnect->setEnabled(true);
        ui->pushButtonDisconnect->setDisabled(true);
        ui->pushButtonPublish->setDisabled(true);
        ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);");
        ui->pushButtonSub->setDisabled(true);
        ui->pushButtonUnsub->setDisabled(true);
        ui->pushButtonSub->setStyleSheet("background-color: rgb(225, 225, 225);");
        ui->listWidgetStatus->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Mqtt broker is disconnected.");
    }
}

void MqttTest::receiveSubState(int state)
{
    if (state == 1)
    {
        isSub = true;
        ui->pushButtonSub->setDisabled(true);
        ui->pushButtonUnsub->setEnabled(true);
        ui->pushButtonSub->setStyleSheet("background-color: rgb(100, 255, 100);");
        ui->listWidgetStatus->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Subscribed to topic " + ui->lineEditTopicSub->text());
        ui->labelSubState->setText("Subscribed");
    }
    else
    {
        ui->labelSubState->setText("Unsubscribed");
        isSub = false;
        if (isConnected)
        {
            ui->pushButtonSub->setEnabled(true);
            ui->pushButtonUnsub->setDisabled(true);
            ui->pushButtonSub->setStyleSheet("background-color: rgb(225, 225, 225);");
        }
        else
        {
            ui->pushButtonSub->setDisabled(true);
            ui->pushButtonUnsub->setDisabled(true);
            ui->pushButtonSub->setStyleSheet("background-color: rgb(225, 225, 225);");
        }
    }
}

void MqttTest::receiveSubMsg(QString topic, QString msg)
{
    ui->listWidgetMsgSub->addItem("[Msg]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                  + "Request " + topic.split("/")[5] + ": " + msg);
}
