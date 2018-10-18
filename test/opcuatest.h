// OpcUaTest class: test OPCUA connection and function
// Author: @ZC
// Date: created on 10 Oct 2018
#ifndef OPCUATEST_H
#define OPCUATEST_H

#include <QWidget>
#include <QOpcUaClient>
#include <QOpcUaNode>
#include <QtOpcUa>
#include <QDateTime>

namespace Ui {
class OpcUaTest;
}

class OpcUaTest : public QWidget
{
    Q_OBJECT

public:
    explicit OpcUaTest(QWidget *parent = nullptr);
    ~OpcUaTest();

private slots:
    void clientConnected();
    void clientDisconnected();
    void clientError(QOpcUaClient::ClientError error);
    void clientState(QOpcUaClient::ClientState state);
    void node1Updated(QOpcUa::NodeAttribute attr, const QVariant &value);
    void node2Updated(QOpcUa::NodeAttribute attr, const QVariant &value);
    void enableMonitoringFinished(QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status);
    void nodeWritten(QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status);

    void on_pushButtonConnect_clicked();
    void on_pushButtonDisconnect_clicked();
    void on_pushButtonMonitor_clicked();
    void on_pushButtonWrite_clicked();

private:
    Ui::OpcUaTest *ui;
    QOpcUaProvider *provider;
    QOpcUaClient *opcuaClient;
    QOpcUaNode *node1;
    QOpcUaNode *node2;
    QOpcUaNode *node3;
    bool connected = false;

    void initSetup();
    void connectToServer();
    void diconnectToServer();
    void node3Write();
};

#endif // OPCUATEST_H
