// LableDisplayer class: subclass of QLabel, more flexible to act on image
// Author: @zichun
// Date: created on 20 Jun 2018
#ifndef LABELDISPLAYER_H
#define LABELDISPLAYER_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QDebug>

class LabelDisplayer : public QLabel
{
    Q_OBJECT

public:
    LabelDisplayer(QWidget *parent = 0);
    ~LabelDisplayer();
    bool startLine1 = false;
    bool startLine2 = false;

protected:
    void mouseMoveEvent(QMouseEvent *mouseQEvent);
    void paintEvent(QPaintEvent *paintQEvent);
    void mousePressEvent(QMouseEvent *mouseQEvent);
    void keyPressEvent(QKeyEvent *keyQEvent);
    void drawLineTo(const QPoint &endPoint);
    void mouseReleaseEvent(QMouseEvent *mouseQEvent);

signals:
    void sendMousePosition(QPoint&);

private:
    QPoint tempPoint, firstPoint, secondPoint, thirdPoint;
    bool firstPointSaved=false, secondPointSaved=false, thirdPointSaved=false;
    bool startDrawing = false;

};

#endif // LABELDISPLAYER_H
