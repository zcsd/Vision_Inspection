#include "labeldisplayer.h"

LabelDisplayer::LabelDisplayer(QWidget* parent) : QLabel(parent)
{

}

LabelDisplayer::~LabelDisplayer()
{

}

void LabelDisplayer::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    QPoint mousePos = mouseEvent->pos();

    if (mousePos.x() <= this->size().width() && mousePos.y() <= this->size().height()) {
        if (mousePos.x() > 0 && mousePos.y() > 0) {
            emit sendMousePosition(mousePos);
        }
    }
}
