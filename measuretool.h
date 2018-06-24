#ifndef MEASURETOOL_H
#define MEASURETOOL_H

#include <QObject>

class MeasureTool : public QObject
{
    Q_OBJECT
public:
    explicit MeasureTool(QObject *parent = nullptr);

signals:

public slots:
};

#endif // MEASURETOOL_H