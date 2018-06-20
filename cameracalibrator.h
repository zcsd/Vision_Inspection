#ifndef CAMERACALIBRATOR_H
#define CAMERACALIBRATOR_H

#include <QObject>
#include <QWidget>
#include <QDebug>

class CameraCalibrator : public QWidget
{
    Q_OBJECT
public:
    explicit CameraCalibrator(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // CAMERACALIBRATOR_H
