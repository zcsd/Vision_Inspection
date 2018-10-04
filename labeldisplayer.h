// LableDisplayer class: subclass of QLabel, more flexible to act on image
// Author: @ZC
// Date: created on 20 Jun 2018
#ifndef LABELDISPLAYER_H
#define LABELDISPLAYER_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QLineF>
#include <QDebug>


class LabelDisplayer : public QLabel
{
    Q_OBJECT

public:
    LabelDisplayer(QWidget *parent = nullptr);
    ~LabelDisplayer();
    void startMCalibration();
    void redoMCalibration();
    void finishMCalibration();
    double getAverageDistance();
    void startManualRuler(double& ppmm);
    void stopManualRuler();

protected:
    void mouseMoveEvent(QMouseEvent *mouseQEvent);
    void paintEvent(QPaintEvent *paintQEvent);
    void mousePressEvent(QMouseEvent *mouseQEvent);
    void keyPressEvent(QKeyEvent *keyQEvent);
    void mouseReleaseEvent(QMouseEvent *mouseQEvent);

signals:
    void sendMousePosition(QPoint&);
    void sendMousePressedPosition(QPoint&);

public slots:
    void receiveCaliCommand(QString command);

private:
    bool selectStart = false;
    double newPPMM = 0.0;
    bool startManualRulerFlag = false, finishManualRulerFlag = true;
    bool startManuCalibration = false, finishManuCalibration = false;
    QPoint tempPoint, firstPoint, secondPoint, thirdPoint;
    bool firstPointSaved=false, secondPointSaved=false, thirdPointSaved=false;

};

#endif // LABELDISPLAYER_H
