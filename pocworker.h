#ifndef POCWORKER_H
#define POCWORKER_H

#include <QObject>
#include <QDebug>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QOpcUaClient>
#include <QOpcUaNode>
#include <QtOpcUa>

class PoCWorker : public QObject
{
    Q_OBJECT
public:
    explicit PoCWorker(QObject *parent = nullptr);
    ~PoCWorker();


signals:
    void sendAuthResult(int, QString, int);
    void readyToGetHMIAuth();
    void authResultWrittenToOpcUa();

public slots:

private slots:
    void opcuaConnected();
    void opcuaDisconnected();
    void opcuaError(QOpcUaClient::ClientError error);
    void opcuaState(QOpcUaClient::ClientState state);
    void enableMonitoringFinished(QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status);

    void prepareToGetHMIAuth();
    void writeAuthResultToOpcua(int isAuth, QString displayUserName, int accessLevel);
    void finishWrittenToOpcUa();

private:
    QNetworkAccessManager *httpRest;
    bool requestToLogin = false, hmiUsernameReady = false, hmiPasswordReady = false;
    QString hmiUsername, hmiPassword;

    QOpcUaProvider *opcuaProvider;
    QOpcUaClient *opcuaClient;
    QOpcUaNode *hmiLoginRequestNodeRW;
    QOpcUaNode *usernameNodeR;
    QOpcUaNode *passwordNodeR;
    QOpcUaNode *authRightNodeW;
    QOpcUaNode *displayUsernameNodeW;
    QOpcUaNode *accessLevelNodeW;
    bool isOpcUaConnected = false;
    bool authRightWritten = false, displayUsernameWritten = false, accessLevelWritten = false;

    void initSetup();
    void connectToOPCUAServer();
    void diconnectToOPCUAServer();
    void getHMILoginAuth(QString username, QString password, QString service = "factory");

};

#endif // POCWORKER_H
