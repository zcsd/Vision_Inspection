#include "opcuatest.h"
#include "ui_opcuatest.h"

OpcUaTest::OpcUaTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OpcUaTest)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);

    initSetup();
}

OpcUaTest::~OpcUaTest()
{
    delete ui;
}

void OpcUaTest::initSetup()
{
    provider = new QOpcUaProvider(this);
    ui->comboBoxProvider->addItems(provider->availableBackends());

    ui->pushButtonConnect->setDisabled(connected);
    ui->pushButtonDisconnect->setEnabled(connected);
    ui->pushButtonMonitor->setEnabled(connected);
    ui->pushButtonWrite->setEnabled(connected);

    connect(ui->listWidgetLog->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            ui->listWidgetLog, SLOT(scrollToBottom()));
}

void OpcUaTest::connectToServer()
{
    opcuaClient = provider->createClient(ui->comboBoxProvider->itemText(ui->comboBoxProvider->currentIndex()));

    if (!opcuaClient)
    {
        ui->listWidgetLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Fail to connect to OPCUA Server");
        return;
    }

    connect(opcuaClient, &QOpcUaClient::connected, this, &OpcUaTest::clientConnected);
    connect(opcuaClient, &QOpcUaClient::disconnected, this, &OpcUaTest::clientDisconnected);
    connect(opcuaClient, &QOpcUaClient::errorChanged, this, &OpcUaTest::clientError);
    connect(opcuaClient, &QOpcUaClient::stateChanged, this, &OpcUaTest::clientState);

    opcuaClient->connectToEndpoint(ui->lineEditURL->text());
}

void OpcUaTest::diconnectToServer()
{
    if (connected)
    {
        opcuaClient->disconnectFromEndpoint();
    }
}

void OpcUaTest::node3Write()
{
    QString msg = ui->lineEditNode3Write->text();

    if (node3)
    {
        uint16_t test = 1;
       if (node3->writeAttribute(QOpcUa::NodeAttribute::Value, msg.toUInt(), QOpcUa::UInt16))
       {
           ui->listWidgetLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                             + "Write " + msg + " to " + node2->nodeId());
       }
    }
}

void OpcUaTest::clientConnected()
{
    connected = true;
    ui->pushButtonConnect->setStyleSheet("background-color: rgb(100, 255, 100);");
    ui->pushButtonConnect->setDisabled(connected);
    ui->pushButtonDisconnect->setEnabled(connected);
    ui->pushButtonMonitor->setEnabled(connected);
    ui->pushButtonWrite->setEnabled(connected);
}

void OpcUaTest::clientDisconnected()
{
    connected = false;
    opcuaClient->deleteLater();
    ui->labelShowNode1->clear();
    ui->labelShowNode2->clear();
    ui->lineEditNode3Write->clear();
    ui->pushButtonConnect->setStyleSheet("background-color: rgb(225, 225, 225);");
    ui->pushButtonConnect->setDisabled(connected);
    ui->pushButtonDisconnect->setEnabled(connected);
    ui->pushButtonMonitor->setEnabled(connected);
    ui->pushButtonWrite->setEnabled(connected);
}

void OpcUaTest::clientError(QOpcUaClient::ClientError error)
{
    ui->listWidgetLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                      + "Client error: " + error);
}

void OpcUaTest::clientState(QOpcUaClient::ClientState state)
{
    if (state == QOpcUaClient::ClientState::Connected)
    {
        ui->listWidgetLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Connected to server succussifully.");
    }
    else if (state == QOpcUaClient::ClientState::Connecting)
    {
        ui->listWidgetLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Client is in connecting state.");
        connected = false;
    }
    else if (state == QOpcUaClient::ClientState::Disconnected)
    {
        ui->listWidgetLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Disconnected to server.");
        connected = false;
    }
}

void OpcUaTest::node1Updated(QOpcUa::NodeAttribute attr, const QVariant &value)
{
    Q_UNUSED(attr);
    //qDebug() << "Read Node1:" << value.toInt();
    ui->labelShowNode1->setText(value.toString());
}

void OpcUaTest::node2Updated(QOpcUa::NodeAttribute attr, const QVariant &value)
{
    Q_UNUSED(attr);
    //qDebug() << "Read Node2:" << value.toString();
    ui->labelShowNode2->setText(value.toString());
}

void OpcUaTest::enableMonitoringFinished(QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
{
    Q_UNUSED(attr);
    if (!sender())
    {
        return;
    }

    if (status == QOpcUa::UaStatusCode::Good)
    {
        ui->listWidgetLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Monitoring successfully enabled for " + qobject_cast<QOpcUaNode *>(sender())->nodeId());
    }
    else
    {
        ui->listWidgetLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Failed to enable monitoring for " + qobject_cast<QOpcUaNode *>(sender())->nodeId());
    }
}

void OpcUaTest::nodeWritten(QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
{
    if (attr == QOpcUa::NodeAttribute::Value && status == QOpcUa::UaStatusCode::Good)
    {
        qDebug() << "Write successfully";
    }
    else if (attr == QOpcUa::NodeAttribute::Value && status != QOpcUa::UaStatusCode::Good)
    {
        qDebug() << "Failed to write";
        ui->listWidgetLog->addItem("[Info]    " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss    ")
                                          + "Failed to write attribute");
    }
}

void OpcUaTest::on_pushButtonConnect_clicked()
{
    connectToServer();
}

void OpcUaTest::on_pushButtonDisconnect_clicked()
{
    diconnectToServer();
}

void OpcUaTest::on_pushButtonMonitor_clicked()
{
    if (connected)
    {
        node1 = opcuaClient->node(ui->lineEditnode1->text());
        //connect(node1, &QOpcUaNode::attributeUpdated, this, &OpcUaTest::node1Updated);
        connect(node1, &QOpcUaNode::attributeUpdated, this, [this](QOpcUa::NodeAttribute attr, const QVariant &value)
        {
            Q_UNUSED(attr);
            //qDebug() << "Read Node1:" << value.toInt();
            ui->labelShowNode1->setText(value.toString());
        });
        node1->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        QObject::connect(node1, &QOpcUaNode::enableMonitoringFinished, this, &OpcUaTest::enableMonitoringFinished);

        node2 = opcuaClient->node(ui->lineEditNode2->text());
        connect(node2, &QOpcUaNode::attributeUpdated, this, &OpcUaTest::node2Updated);
        node2->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        QObject::connect(node2, &QOpcUaNode::enableMonitoringFinished, this, &OpcUaTest::enableMonitoringFinished);
    }
}

void OpcUaTest::on_pushButtonWrite_clicked()
{
    if (connected)
    {
        node3 = opcuaClient->node(ui->lineEditNode3->text());
        QObject::connect(node3, &QOpcUaNode::attributeWritten, this, &OpcUaTest::nodeWritten);
        node3Write();
    }
}
