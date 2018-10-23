#include "pocworker.h"

PoCWorker::PoCWorker(QObject *parent) : QObject(parent)
{
    initSetup();
}

PoCWorker::~PoCWorker()
{
    diconnectToOPCUAServer();
    //delete opcuaClient;
    //delete opcuaProvider;
}

void PoCWorker::initSetup()
{
    connect(this, SIGNAL(readyToGetHMIAuth()), this, SLOT(prepareToGetHMIAuth()));
    connect(this, SIGNAL(sendAuthResult(int, QString, QString)), this, SLOT(writeAuthResultToOpcua(int, QString, QString)));
    connect(this, SIGNAL(authResultWrittenToOpcUa()), this, SLOT(finishWrittenToOpcUa()));

    opcuaProvider = new QOpcUaProvider(this);
    connectToOPCUAServer();

    if (isOpcUaConnected)
    {
        authRightNodeW = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.autho_approve"); //int16
        connect(authRightNodeW, &QOpcUaNode::attributeWritten, this, [this](QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
        {
            if (attr == QOpcUa::NodeAttribute::Value && status == QOpcUa::UaStatusCode::Good)
            {
                qDebug() << "Write auth right to opcua server successfully.";
                authRightWritten = true;
            }
            else if (attr == QOpcUa::NodeAttribute::Value && status != QOpcUa::UaStatusCode::Good)
            {
                qDebug() << "Failed to write auth right to opcua server.";
                authRightWritten = false;
            }
        });

        displayUsernameNodeW = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.autho_name"); //string
        connect(displayUsernameNodeW, &QOpcUaNode::attributeWritten, this, [this](QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
        {
            if (attr == QOpcUa::NodeAttribute::Value && status == QOpcUa::UaStatusCode::Good)
            {
                qDebug() << "Write display-username to opcua server successfully.";
                displayUsernameWritten = true;
            }
            else if (attr == QOpcUa::NodeAttribute::Value && status != QOpcUa::UaStatusCode::Good)
            {
                qDebug() << "Failed to write display-username to opcua server.";
                displayUsernameWritten = false;
            }
        });

        accessLevelNodeW = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.autho_accessLevel");//int16
        connect(accessLevelNodeW, &QOpcUaNode::attributeWritten, this, [this](QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
        {
            if (attr == QOpcUa::NodeAttribute::Value && status == QOpcUa::UaStatusCode::Good)
            {
                qDebug() << "Write access level to opcua server successfully.";
                accessLevelWritten = true;
            }
            else if (attr == QOpcUa::NodeAttribute::Value && status != QOpcUa::UaStatusCode::Good)
            {
                qDebug() << "Failed to write access level to opcua server.";
                accessLevelWritten = false;
            }
        });

        usernameNodeR = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.username"); // string
        //usernameNodeR->disableMonitoring(QOpcUa::NodeAttribute::Value);
        connect(usernameNodeR, &QOpcUaNode::attributeUpdated, this, [this](QOpcUa::NodeAttribute attr, const QVariant &value)
        {
            Q_UNUSED(attr);
            qDebug() << "Read username node:" << value.toString();
            if (value.toString().length() >= 2)
            {
                hmiUsername = value.toString();
                hmiUsernameReady = true;
                emit readyToGetHMIAuth();
            }
        });

        passwordNodeR = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.password"); // string
        //passwordNodeR->disableMonitoring(QOpcUa::NodeAttribute::Value);
        connect(passwordNodeR, &QOpcUaNode::attributeUpdated, this, [this](QOpcUa::NodeAttribute attr, const QVariant &value)
        {
            Q_UNUSED(attr);
            qDebug() << "Read password node:" << value.toString();
            if (value.toString().length() >= 2)
            {
                hmiPassword = value.toString();
                hmiPasswordReady = true;
                emit readyToGetHMIAuth();
            }
        });

        // HMI Login Request Node (read and write)
        hmiLoginRequestNodeRW = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.autho_request"); // int16
        connect(hmiLoginRequestNodeRW, &QOpcUaNode::attributeUpdated, this, [this](QOpcUa::NodeAttribute attr, const QVariant &value)
        {
            Q_UNUSED(attr);
            qDebug() << "Read autho-requset node:" << value.toInt();
            if (value.toInt() == 1)
            {
                requestToLogin = true;
                usernameNodeR->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
                passwordNodeR->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
            }
        });
        connect(hmiLoginRequestNodeRW, &QOpcUaNode::enableMonitoringFinished, this, &PoCWorker::enableMonitoringFinished);
        hmiLoginRequestNodeRW->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
        connect(hmiLoginRequestNodeRW, &QOpcUaNode::attributeWritten, this, [this](QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
        {
            if (attr == QOpcUa::NodeAttribute::Value && status == QOpcUa::UaStatusCode::Good)
            {
                qDebug() << "Write(Change) auth request to opcua server successfully.";
                usernameNodeR->disableMonitoring(QOpcUa::NodeAttribute::Value);
                passwordNodeR->disableMonitoring(QOpcUa::NodeAttribute::Value);
                requestToLogin = false;
                authRightWritten = false;
                displayUsernameWritten = false;
                accessLevelWritten = false;
                hmiUsernameReady = false;
                hmiPasswordReady = false;
                hmiUsername = "";
                hmiPassword = "";
                qDebug() << "Finish HMI login auth process.";
            }
            else if (attr == QOpcUa::NodeAttribute::Value && status != QOpcUa::UaStatusCode::Good)
            {
                qDebug() << "Failed to write auth request to opcua server.";
            }
        });
    }
}

void PoCWorker::connectToOPCUAServer()
{
    const static QUrl opcuaServer(QLatin1String("opc.tcp://172.19.80.34:4840"));
    opcuaClient = opcuaProvider->createClient(opcuaProvider->availableBackends()[0]);

    if (!opcuaClient)
    {
        qDebug() << "Fail to create OPCUA client.";
        return;
    }

    connect(opcuaClient, &QOpcUaClient::connected, this, &PoCWorker::opcuaConnected);
    connect(opcuaClient, &QOpcUaClient::disconnected, this, &PoCWorker::opcuaDisconnected);
    connect(opcuaClient, &QOpcUaClient::errorChanged, this, &PoCWorker::opcuaError);
    connect(opcuaClient, &QOpcUaClient::stateChanged, this, &PoCWorker::opcuaState);

    opcuaClient->connectToEndpoint(opcuaServer);
}

void PoCWorker::diconnectToOPCUAServer()
{
    if (isOpcUaConnected)
    {
        opcuaClient->disconnectFromEndpoint();
    }
}


void PoCWorker::opcuaConnected()
{
    isOpcUaConnected = true;
}

void PoCWorker::opcuaDisconnected()
{
    isOpcUaConnected = false;
    opcuaClient->deleteLater();
}

void PoCWorker::opcuaError(QOpcUaClient::ClientError error)
{
    qDebug() << "OPCUA Client Error:" << error;
}

void PoCWorker::opcuaState(QOpcUaClient::ClientState state)
{
    if (state == QOpcUaClient::ClientState::Connected)
    {
        qDebug() << "Successfully connected to OPCUA server";
        isOpcUaConnected = true;
    }
    else if (state == QOpcUaClient::ClientState::Connecting)
    {
        qDebug() << "Trying to connect OPCUA server now.";
        isOpcUaConnected = false;
    }
    else if (state == QOpcUaClient::ClientState::Disconnected)
    {
        qDebug() << "Disconnected to OPCUA server.";
        isOpcUaConnected = false;
    }
}

void PoCWorker::enableMonitoringFinished(QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
{
    Q_UNUSED(attr);
    if (!sender())
    {
        return;
    }

    if (status == QOpcUa::UaStatusCode::Good)
    {
        qDebug() << "Monitoring successfully enabled for" << qobject_cast<QOpcUaNode *>(sender())->nodeId();
    }
    else
    {
        qDebug() << "Failed to enable monitoring for" << qobject_cast<QOpcUaNode *>(sender())->nodeId();
    }
}

void PoCWorker::prepareToGetHMIAuth()
{
    if (requestToLogin && hmiUsernameReady && hmiPasswordReady)
    {
        getHMILoginAuth(hmiUsername, hmiPassword, "factory");
    }
}

void PoCWorker::writeAuthResultToOpcua(int isAuth, QString displayUserName, QString accessLevel)
{
    if (authRightNodeW)
    {
       authRightNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, isAuth, QOpcUa::Int16);
    }
    if (displayUsernameNodeW)
    {
        displayUsernameNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, displayUserName, QOpcUa::String);
    }
    if (accessLevelNodeW)
    {
        accessLevelNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, accessLevel, QOpcUa::String);
    }
}

void PoCWorker::finishWrittenToOpcUa()
{
    if (authRightWritten && displayUsernameWritten && accessLevelWritten)
    {
        if (hmiLoginRequestNodeRW)
        {
            hmiLoginRequestNodeRW->writeAttribute(QOpcUa::NodeAttribute::Value, 0, QOpcUa::Int16);
        }
    }
}

void PoCWorker::getHMILoginAuth(QString username, QString password, QString service)
{
    //use http rest api to get auth right, with json
    const static QUrl authServer(QLatin1String("http://sat-mes/server/auth/authenticate"));

    httpRest = new QNetworkAccessManager(this);
    QNetworkRequest request(authServer);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject obj
    {
        {QStringLiteral("username"), username},
        {QStringLiteral("password"), password},
        {QStringLiteral("service"), service},
    };

    QNetworkReply *reply = httpRest->post(request, QJsonDocument(obj).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonObject obj = doc.object();

        //qDebug() << obj.value(QLatin1String("result")).toInt();
        //qDebug() << obj.value(QLatin1String("displayName")).toString();
        //qDebug() << obj.value(QLatin1String("rights")).toObject().value(QLatin1String("mespoc")).toObject().value(QLatin1String("mespoc")).toObject().value(QLatin1String("User")).toString();
        int isAuth = obj.value(QLatin1String("result")).toInt();
        QString displayUserName = obj.value(QLatin1String("displayName")).toString();
        QString accessLevel = obj.value(QLatin1String("rights")).toObject().value(QLatin1String("mespoc")).toObject().value(QLatin1String("mespoc")).toObject().value(QLatin1String("User")).toString();

        qDebug() << isAuth << displayUserName << accessLevel;
        if (isAuth == 1)
        {
            emit sendAuthResult(isAuth, displayUserName, accessLevel);
        }
        else
        {
            emit sendAuthResult(isAuth, "NA", "8");
        }

    });

    delete httpRest;
}

