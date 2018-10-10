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
    void connectToBroker(QString _ip = "172.19.80.25", QString _mqttPort = "1883",
                 QString _username = "tenant@thingsboard.org", QString _password = "tenant",
                 QString _tokenPort = "8080", QString _deviceID = "624facf0-cb6e-11e8-8891-05a8a3fcf36e");
    void disconnect();
    void publish(QString topic, QString msg, int QoS = 0, bool isRetain = false);
    void subscribe(QString topic, int QoS = 0);
    void unsubscribe(QString topic);
    void keepAlive(int interval);

signals:
    void sendSubMsg(QString topic, QString msg);
    void sendConState(int state);

public slots:

private slots:
    void updateConStateChange();
    void brokerDisconnected();
    void updateSubState(QMqttSubscription::SubscriptionState state);
    void updateSubMessage(const QMqttMessage &msg);

private:
    QMqttClient *mqttClient;
    QString ip, mqttPort, username, password, tokenPort, deviceID;
    void initSetup();
    QString getJWT();
    QString getToken();
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
    const char* qstr2char(QString in);
};

#endif // MQTTCLIENT_H
