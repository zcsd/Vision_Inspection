#ifndef CALIBRATORFORM_H
#define CALIBRATORFORM_H

#include <QWidget>
#include <QDebug>

namespace Ui {
class CalibratorForm;
}

class CalibratorForm : public QWidget
{
    Q_OBJECT

public:
    explicit CalibratorForm(QWidget *parent = 0);
    ~CalibratorForm();

signals:
    void sendTest();

private slots:
    void on_pushButton_clicked();

private:
    Ui::CalibratorForm *ui;
};

#endif // CALIBRATORFORM_H
