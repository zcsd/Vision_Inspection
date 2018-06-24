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
    double realDistance = QLineF(firstPoint, secondPoint).length() / newPPMM;
    QFont font = painter.font() ;
    /* twice the size than the current font size */
    font.setPointSize(15);
    /* set the modified font to the painter */
    painter.setFont(font);

    if (startManuCalibration && !finishManuCalibration) {
        if (!firstPointSaved || !secondPointSaved || !thirdPointSaved) {
            painter.setPen(QPen(Qt::blue, 2, Qt::SolidLine));
            painter.drawEllipse(tempPoint, 15, 15);
            painter.setPen(QPen(Qt::blue, 1, Qt::SolidLine));
            painter.drawLine(QPoint(tempPoint.x()-5, tempPoint.y()), QPoint(tempPoint.x()+5, tempPoint.y()));
            painter.drawLine(QPoint(tempPoint.x(), tempPoint.y()-5), QPoint(tempPoint.x(), tempPoint.y()+5));
            //painter.drawEllipse(tempPoint, 1, 1);
        }
        if (firstPointSaved) {
            painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
            painter.drawEllipse(firstPoint, 10, 10);
            painter.setPen(QPen(Qt::red, 1, Qt::SolidLine));
            painter.drawLine(QPoint(firstPoint.x()-4, firstPoint.y()), QPoint(firstPoint.x()+4, firstPoint.y()));
            painter.drawLine(QPoint(firstPoint.x(), firstPoint.y()-4), QPoint(firstPoint.x(), firstPoint.y()+4));
            //painter.drawEllipse(firstPoint, 1, 1);
        }
        if (secondPointSaved && firstPointSaved) {
            painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
            painter.drawEllipse(secondPoint, 10, 10);
            painter.setPen(QPen(Qt::red, 1, Qt::SolidLine));
            painter.drawLine(QPoint(secondPoint.x()-4, secondPoint.y()), QPoint(secondPoint.x()+4, secondPoint.y()));
            painter.drawLine(QPoint(secondPoint.x(), secondPoint.y()-4), QPoint(secondPoint.x(), secondPoint.y()+4));
            painter.setPen(QPen(Qt::green, 2, Qt::DotLine));
            painter.drawLine(firstPoint, secondPoint);
            painter.setPen(QPen(Qt::magenta, 2, Qt::SolidLine));
            if (!startManualRulerFlag)
                painter.drawText(QLineF(firstPoint, secondPoint).center(), QString::number(QLineF(firstPoint, secondPoint).length(), 'f', 1));
            //painter.drawEllipse(secondPoint, 1, 1);
            if (startManualRulerFlag) {
                //startManualRulerFlag = false;
                //finishManualRulerFlag = true;
                painter.drawText(QLineF(firstPoint, secondPoint).center(), QString::number(realDistance, 'f', 2)+"mm");
            }
        }
        if (thirdPointSaved && secondPointSaved) {
            painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
            painter.drawEllipse(thirdPoint, 10, 10);
            painter.setPen(QPen(Qt::red, 1, Qt::SolidLine));
            painter.drawLine(QPoint(thirdPoint.x()-4, thirdPoint.y()), QPoint(thirdPoint.x()+4, thirdPoint.y()));
            painter.drawLine(QPoint(thirdPoint.x(), thirdPoint.y()-4), QPoint(thirdPoint.x(), thirdPoint.y()+4));
            painter.setPen(QPen(Qt::green, 2, Qt::DotLine));
            painter.drawLine(secondPoint, thirdPoint);
            painter.setPen(QPen(Qt::magenta, 2, Qt::SolidLine));
            //painter.drawText(QLineF(secondPoint, thirdPoint).center(), QString::number(QLineF(secondPoint, thirdPoint).length(), 'f', 1));
            if (!startManualRulerFlag)
                painter.drawText(QLineF(secondPoint, thirdPoint).center(), QString::number(QLineF(secondPoint, thirdPoint).length(), 'f', 1));
            //painter.drawEllipse(secondPoint, 1, 1);
            if (startManualRulerFlag) {
                //startManualRulerFlag = false;
                //finishManualRulerFlag = true;
                double realDistance = QLineF(secondPoint, thirdPoint).length() / newPPMM;
                painter.drawText(QLineF(secondPoint, thirdPoint).center(), QString::number(realDistance, 'f', 2)+"mm");
            }
            //painter.drawEllipse(thirdPoint, 1, 1);
        }
    }
}

void LabelDisplayer::mousePressEvent(QMouseEvent *mouseQEvent)
{
    if (mouseQEvent->button() == Qt::LeftButton) {
        setFocus();
        tempPoint = mouseQEvent->pos();
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
    }
}

void LabelDisplayer::mouseReleaseEvent(QMouseEvent *mouseQEvent)
{
    if (mouseQEvent->button() == Qt::LeftButton) {
        update();
    }
}

void LabelDisplayer::startMCalibration()
{
    startManuCalibration = true;
    finishManuCalibration = false;
    firstPointSaved = false;
    secondPointSaved = false;
    thirdPointSaved = false;
    finishManualRulerFlag = true;
    startManualRulerFlag = false;
}

void LabelDisplayer::redoMCalibration()
{
    startManuCalibration = true;
    finishManuCalibration = false;
    firstPointSaved = false;
    secondPointSaved = false;
    thirdPointSaved = false;
    update();
}

void LabelDisplayer::finishMCalibration()
{
    startManuCalibration = false;
    finishManuCalibration = true;
    update();
}

double LabelDisplayer::getAverageDistance()
{
    double d1 = QLineF(firstPoint, secondPoint).length();
    double d2 = QLineF(secondPoint, thirdPoint).length();
    return ( (d1+d2)/2 );
}

void LabelDisplayer::startManualRuler(double& ppmm)
{
    update();

    newPPMM = ppmm;
    startMCalibration();
    finishManualRulerFlag = false;
    startManualRulerFlag = true;
}
