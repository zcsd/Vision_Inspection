// HttpAuth class: request auth from app server using http rest api
// Author: @ZC
// Date: created on 22 Oct 2018
#ifndef HTTPAUTH_H
#define HTTPAUTH_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>

class HttpAuth : public QObject
{
    Q_OBJECT
public:
    explicit HttpAuth(QObject *parent = nullptr);
    void getAuth(QString username, QString password)
    {
        nam = new QNetworkAccessManager(this);
        QNetworkRequest request(QUrl("http://sat-mes/server/auth/authenticate"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject obj{
            {QStringLiteral("username"), username},
            {QStringLiteral("password"), password},
            {QStringLiteral("service"), "factory"},
        };

        QNetworkReply *reply = nam->post(request, QJsonDocument(obj).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            reply->deleteLater();
            const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            const QJsonObject obj = doc.object();

            qDebug() << obj.value(QLatin1String("result")).toInt();
            qDebug() << obj.value(QLatin1String("displayName")).toString();
            qDebug() << obj.value(QLatin1String("rights")).toObject().value(QLatin1String("mespoc")).toObject().value(QLatin1String("mespoc")).toObject().value(QLatin1String("User")).toString();

        });
        delete nam;
    };

signals:

public slots:

private:
    QNetworkAccessManager *nam;
    //const static QString contentTypeJson(QLatin1String("application/json"));

};

#endif // HTTPAUTH_H
