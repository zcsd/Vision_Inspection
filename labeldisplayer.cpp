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
    painter.setPen(QPen(Qt::blue, 2, Qt::SolidLine));
    if ( (!firstPointSaved || !secondPointSaved || !thirdPointSaved) && startDrawing ) {
        painter.drawEllipse(tempPoint, 15, 15);
        painter.drawEllipse(tempPoint, 1, 1);
    }
    painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
    if (firstPointSaved) {
        painter.drawEllipse(firstPoint, 10, 10);
        painter.drawEllipse(firstPoint, 1, 1);
    }
    if (secondPointSaved && firstPointSaved) {
        painter.drawEllipse(secondPoint, 10, 10);
        painter.drawEllipse(secondPoint, 1, 1);
        painter.setPen(QPen(Qt::green, 2, Qt::DotLine));
        painter.drawLine(firstPoint, secondPoint);
    }
    painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
    if (thirdPointSaved && secondPointSaved) {
        painter.drawEllipse(thirdPoint, 10, 10);
        painter.drawEllipse(thirdPoint, 1, 1);
        painter.setPen(QPen(Qt::green, 2, Qt::DotLine));
        painter.drawLine(secondPoint, thirdPoint);
    }
    qDebug() << "paint";
}

void LabelDisplayer::drawLineTo(const QPoint &endPoint)
{
    qDebug() << "draw";

    //lastPoint = endPoint;
    //secondPoint = lastPoint;

    //update();
}

void LabelDisplayer::mousePressEvent(QMouseEvent *mouseQEvent)
{
    if (mouseQEvent->button() == Qt::LeftButton) {
        setFocus();
        tempPoint = mouseQEvent->pos();
        startDrawing = true;
        qDebug() << "Left press" << tempPoint;
    }
}

void LabelDisplayer::keyPressEvent(QKeyEvent *keyQEvent)
{
    if (keyQEvent->key() == Qt::Key_Space) {
        if (!firstPointSaved) {
            firstPoint = tempPoint;
            firstPointSaved = true;
        } else if (!secondPointSaved) {
            secondPoint = tempPoint;
            secondPointSaved = true;
        } else if (!thirdPointSaved) {
            thirdPoint = tempPoint;
            thirdPointSaved = true;
        }
        update();
        qDebug() << "Enter press" << firstPoint;
    }
}

void LabelDisplayer::mouseReleaseEvent(QMouseEvent *mouseQEvent)
{
    if (mouseQEvent->button() == Qt::LeftButton) {
        update();
        qDebug() << "release";
    }
}
