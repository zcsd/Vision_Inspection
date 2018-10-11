#include "mqttclient.h"

MqttClient::MqttClient(QObject *parent) : QObject(parent)
{
    mqttClient = new QMqttClient(this);
    initSetup();
}

MqttClient::~MqttClient()
{
    delete mqttClient;
}

void MqttClient::initSetup()
{
    connect(mqttClient, &QMqttClient::stateChanged, this, &MqttClient::updateConStateChange);
    connect(mqttClient, &QMqttClient::disconnected, this, &MqttClient::brokerDisconnected);
    /*
    connect(mqttClient, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic)
    {
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" Received Topic: ")
                    + topic.name()
                    + QLatin1String(" Message: ")
                    + message
                    + QLatin1Char('\n');
        qDebug() << content;
    });*/
    /*
    connect(mqttClient, &QMqttClient::pingResponseReceived, this, [this]()
    {
        const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(" PingResponse")
                    + QLatin1Char('\n');
        qDebug() << content;
    });*/

    updateConStateChange();
}

void MqttClient::connectToBroker(QString _ip, QString _mqttPort, QString _username,
                         QString _password, QString _deviceID, QString _tokenPort)
{
    ip = _ip;
    mqttPort = _mqttPort;
    username = _username;
    password = _password;
    tokenPort = _tokenPort;
    deviceID = _deviceID;

    if (mqttClient->state() == QMqttClient::Disconnected)
    {
        mqttClient->setHostname(ip);
        mqttClient->setPort(mqttPort.toInt());
        mqttClient->setUsername(getToken());
        mqttClient->connectToHost();
    }
}

void MqttClient::disconnect()
{
    if (mqttClient->state() != QMqttClient::Disconnected)
    {
        mqttClient->disconnectFromHost();
    }
}

bool MqttClient::isConnected()
{
    return connectionStatus;
}

void MqttClient::publish(QString topic, QString msg, int QoS, bool isRetain)
{
    if (mqttClient->state() == QMqttClient::Connected)
    {
        mqttClient->publish(topic, msg.toUtf8(), QoS, isRetain);
    }
    else
    {
        qDebug() << "Publish error: no connection.";
    }
}

void MqttClient::subscribe(QString topic, int QoS)
{
    if (mqttClient->state() == QMqttClient::Connected)
    {
        auto subscription = mqttClient->subscribe(topic, QoS);
        if (!subscription)
        {
            qDebug() << "Subscription Failure, but have connection";
            return;
        }
        currentTopic = subscription->topic().filter();
        updateSubState(subscription->state());
        connect(subscription, &QMqttSubscription::messageReceived, this, &MqttClient::updateSubMessage);
        connect(subscription, &QMqttSubscription::stateChanged, this, &MqttClient::updateSubState);
        connect(this, SIGNAL(sendUnsubCmd()), subscription, SLOT(unsubscribe()));
    }
    else
    {
        qDebug() << "Subscription error: no connection.";
    }
}

void MqttClient::unsubscribe(QString topic)
{
    if (mqttClient->state() == QMqttClient::Connected)
    {
        emit sendUnsubCmd();
    }
}

void MqttClient::keepAlive(int interval)
{
    if (mqttClient->state() == QMqttClient::Connected)
    {
        mqttClient->setKeepAlive(interval);
    }
}

void MqttClient::updateConStateChange()
{   /*
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(mqttClient->state())
                    + QLatin1Char('\n');
    qDebug() << content;*/

    if (mqttClient->state() == QMqttClient::Connected)
    {
        connectionStatus = true;
        emit sendConState(1);
    }
    else
    {
        connectionStatus = false;
        emit sendConState(0);
    }
}

void MqttClient::brokerDisconnected()
{
    //qDebug() << "Broker disconnnected";
    connectionStatus = false;
}

void MqttClient::updateSubState(QMqttSubscription::SubscriptionState state)
{
    QString qsubState;
    int subState = 0;

    switch (state) {
    case QMqttSubscription::Unsubscribed:
        qsubState = "Unsubscribed";
        break;
    case QMqttSubscription::SubscriptionPending:
        qsubState = "Pending";
        break;
    case QMqttSubscription::Subscribed:
        qsubState = "Subscribed";
        break;
    case QMqttSubscription::Error:
        qsubState = "Error";
        break;
    default:
        qsubState = "Unknown";
        break;
    }

    if (qsubState == "Subscribed")
    {
        subState = 1;
    }
    else
    {
        subState = 0;
    }

    emit sendSubState(subState);
    //qDebug() << "subscription state changes to:" << subState;
}

void MqttClient::updateSubMessage(const QMqttMessage &msg)
{
    //qDebug() << "Message Receive:" << msg.payload();
    emit sendSubMsg(currentTopic, QString::fromUtf8(msg.payload()));
}

QString MqttClient::getJWT()
{
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    QString JWT;

    QString jwtURL = "http://" + ip + ":" + tokenPort + "/api/auth/login";
    QString postfield = "{\"username\":\"" + username + "\",\"password\":\"" + password + "\"}";

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();

    if (curl)
    {
      curl_easy_setopt(curl, CURLOPT_URL, qstr2char(jwtURL));
      //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

      struct curl_slist *chunk = NULL;
      chunk = curl_slist_append(chunk, "Accept: application/json");
      chunk = curl_slist_append(chunk, "Content-Type: application/json");
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

      curl_easy_setopt(curl, CURLOPT_POST, 1);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, qstr2char(postfield));

      res = curl_easy_perform(curl);

      if (res != CURLE_OK)
      {
          qDebug() << curl_easy_strerror(res);

      }

      curl_easy_cleanup(curl);


      JWT = QString::fromStdString(readBuffer).split("\"")[3];

      curl_slist_free_all(chunk);
    }

    curl_global_cleanup();

    return JWT;
}

QString MqttClient::getToken()
{
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    QString token;

    QString tokenURL = "http://" + ip + ":" + tokenPort + "/api/device/" + deviceID + "/credentials";

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();

    if (curl)
    {
      curl_easy_setopt(curl, CURLOPT_URL, qstr2char(tokenURL));
      //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

      struct curl_slist *chunk = nullptr;
      chunk = curl_slist_append(chunk, "Accept: application/json");
      QString auth = "X-Authorization: Bearer " + getJWT();
      chunk = curl_slist_append(chunk, qstr2char(auth));

      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

      res = curl_easy_perform(curl);

      if(res != CURLE_OK)
      {
          qDebug() << curl_easy_strerror(res);
      }

      curl_easy_cleanup(curl);
      curl_slist_free_all(chunk);
      token = QString::fromStdString(readBuffer).split(",")[5].split(":")[1].replace("\"", "");
    }

    curl_global_cleanup();

    return token;
}

size_t MqttClient::writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

const char *MqttClient::qstr2char(QString in)
{
    QByteArray ba = in.toLatin1();
    const char *out = ba.data();

    return out;
}
