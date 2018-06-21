// LableDisplayer class: subclass of QLabel, more flexible to act on image
// Author: @zichun
// Date: created on 20 Jun 2018
#ifndef LABELDISPLAYER_H
#define LABELDISPLAYER_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QMouseEvent>

class LabelDisplayer : public QLabel
{
    Q_OBJECT

public:
    LabelDisplayer(QWidget *parent = 0);
    ~LabelDisplayer();

protected:
    void mouseMoveEvent(QMouseEvent *mouseEvent);

signals:
    void sendMousePosition(QPoint&);
};

#endif // LABELDISPLAYER_H
