// MqttClient class: communicate with IoT thingsboard server
// Author: @ZC
// Date: created on 10 Oct 2018
#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>

#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttMessage>
#include <QtMqtt/QMqttSubscription>

#include <stdio.h>
#include <iostream>
#include <string>

#include <curl/curl.h>

class MqttClient : public QObject
{
    Q_OBJECT
public:
    explicit MqttClient(QObject *parent = nullptr);
    ~MqttClient();
    void connectToBroker(QString _ip, QString _mqttPort, QString _username, QString _password,
                         QString _deviceID, QString _tokenPort = "8080");
    void disconnect();
    bool isConnected();
    void publish(QString topic, QString msg, int QoS = 0, bool isRetain = false);
    void subscribe(QString topic, int QoS = 0);
    void unsubscribe(QString topic);
    void keepAlive(int interval);

signals:
    void sendSubMsg(QString topic, QString msg);
    void sendConState(int state);
    void sendSubState(int state);
    void sendUnsubCmd();

public slots:

private slots:
    void updateConStateChange();
    void brokerDisconnected();
    void updateSubState(QMqttSubscription::SubscriptionState state);
    void updateSubMessage(const QMqttMessage &msg);

private:
    QMqttClient *mqttClient;
    QString ip, mqttPort, username, password, tokenPort, deviceID;
    QString currentTopic;
    bool connectionStatus = false;
    void initSetup();
    QString getJWT();
    QString getToken();
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
    const char* qstr2char(QString in);
};

#endif // MQTTCLIENT_H
