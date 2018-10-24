#include "pocworker.h"

PoCWorker::PoCWorker(QObject *parent) : QObject(parent)
{
    initSetup();
}

PoCWorker::~PoCWorker()
{
    diconnectToOPCUAServer();
    delete opcuaClient;
    delete opcuaProvider;
    delete httpRest;
}

void PoCWorker::initSetup()
{
    connect(this, SIGNAL(readyToGetHMIAuth()), this, SLOT(prepareToGetHMIAuth()));
    connect(this, SIGNAL(sendAuthResult(int, QString, int)), this, SLOT(writeAuthResultToOpcua(int, QString, int)));
    connect(this, SIGNAL(authResultWrittenToOpcUa()), this, SLOT(finishWrittenToOpcUa()));

    opcuaProvider = new QOpcUaProvider(this);
    httpRest = new QNetworkAccessManager(this);

    if (!isOpcUaConnected)
    {
        connectToOPCUAServer();
    }
}

void PoCWorker::connectToOPCUAServer()
{
    const static QUrl opcuaServer(QLatin1String("opc.tcp://172.19.80.34:4840"));
    // default plugin is open62541
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

    opcuaClient->connectToEndpoint(opcuaServer); // connect action
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

    authRightNodeW = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.autho_approve"); //int16
    connect(authRightNodeW, &QOpcUaNode::attributeWritten, this, [this](QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
    {
        if (attr == QOpcUa::NodeAttribute::Value && status == QOpcUa::UaStatusCode::Good)
        {
            qDebug() << "Write autho_approve to opcua server successfully.";
            authRightWritten = true;
            emit authResultWrittenToOpcUa();
        }
        else if (attr == QOpcUa::NodeAttribute::Value && status != QOpcUa::UaStatusCode::Good)
        {
            qDebug() << "Failed to write autho_approve to opcua server.";
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
            emit authResultWrittenToOpcUa();
        }
        else if (attr == QOpcUa::NodeAttribute::Value && status != QOpcUa::UaStatusCode::Good)
        {
            qDebug() << "Failed to write display-username to opcua server.";
            displayUsernameWritten = false;
        }
    });

    accessLevelNodeW = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.autho_accessLevel"); //int16
    connect(accessLevelNodeW, &QOpcUaNode::attributeWritten, this, [this](QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
    {
        if (attr == QOpcUa::NodeAttribute::Value && status == QOpcUa::UaStatusCode::Good)
        {
            qDebug() << "Write accesslevel to opcua server successfully.";
            accessLevelWritten = true;
            emit authResultWrittenToOpcUa();
        }
        else if (attr == QOpcUa::NodeAttribute::Value && status != QOpcUa::UaStatusCode::Good)
        {
            qDebug() << "Failed to write accesslevel to opcua server.";
            accessLevelWritten = false;
        }
    });

    usernameNodeR = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.username"); // string
    connect(usernameNodeR, &QOpcUaNode::attributeUpdated, this, [this](QOpcUa::NodeAttribute attr, const QVariant &value)
    {
        Q_UNUSED(attr);
        qDebug() << "Read username node:" << value.toString().replace(" ", "");
        if (value.toString().length() >= 2)
        {
            hmiUsername = value.toString().replace(" ", "");
            hmiUsernameReady = true;
            emit readyToGetHMIAuth();
        }
    });

    passwordNodeR = opcuaClient->node("ns=2;s=|var|CPS-PCS341MB-DS1.Application.GVL.OPC_Machine_A0001.password"); // string
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
            qDebug() << "Write(Change) auth_request(reset to 0) to opcua server successfully.";
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
            qDebug() << "Failed to write(change) auth_request to opcua server.";
        }
    });
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
        qDebug() << "Successfully connected to OPCUA server.";
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

void PoCWorker::writeAuthResultToOpcua(int isAuth, QString displayUserName, int accessLevel)
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
        accessLevelNodeW->writeAttribute(QOpcUa::NodeAttribute::Value, accessLevel, QOpcUa::Int16);
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
    qDebug() << "Sending http requset to get autho information for PLC HMI...";
    //use http rest api to get auth right, with json
    const static QUrl authServer(QLatin1String("http://sat-mes/server/auth/authenticate"));

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

        int authCode = obj.value(QLatin1String("result")).toInt();
        QString displayUserName = obj.value(QLatin1String("displayName")).toString();
        QString accessLevel = obj.value(QLatin1String("rights")).toObject().value(QLatin1String("mespoc")).toObject().value(QLatin1String("mespoc")).toObject().value(QLatin1String("User")).toString();

        qDebug() << "Get autho result from autho server:" << authCode << displayUserName << accessLevel;
        if (authCode == 1)
        {
            authCode = 8; // int 8 for approve in PLC
            emit sendAuthResult(authCode, displayUserName, accessLevel.toInt());
        }
        else
        {
            authCode = 7; // int 7 for rejected in PLC
            emit sendAuthResult(authCode, "NA", 0);
        }

    });
}

