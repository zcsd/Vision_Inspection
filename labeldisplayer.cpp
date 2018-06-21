#include "labeldisplayer.h"

LabelDisplayer::LabelDisplayer(QWidget* parent) : QLabel(parent)
{

}

LabelDisplayer::~LabelDisplayer()
{

}

void LabelDisplayer::mouseMoveEvent(QMouseEvent *mouseQEvent)
{
    QPoint mousePos = mouseQEvent->pos();

    if (mousePos.x() <= this->size().width() && mousePos.y() <= this->size().height()) {
        if (mousePos.x() > 0 && mousePos.y() > 0) {
            emit sendMousePosition(mousePos);
        }
    }
}

void LabelDisplayer::paintEvent(QPaintEvent *paintQEvent)
{
    // MUST call paintEvent of QLabel parent to display image(background)
    QLabel::paintEvent(paintQEvent);
    QPainter painter(this);
    painter.setPen(QPen(Qt::red,2));
    painter.drawRect(QRect(100,100,100,100));
    //painter.drawLine(QPoint,QPoint);
}
