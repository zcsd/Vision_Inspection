// SettingDialog class for camera setting dialog UI
// Author: @ZC
// Date: created on 27 Jun 2018
#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <QtWidgets>
#include <QStringList>
#include <QDebug>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = 0);
    ~SettingDialog();

private slots:
    void on_pushButtonCancel_clicked();
    void on_pushButtonOK_clicked();

private:
    Ui::SettingDialog *ui;
    QMap<QString, QString> cameraParaMap;
    void initSetup();
    void readConf();
    void updateConf();
    QString getParameters(QString nameParameter);

};

#endif // SETTINGDIALOG_H
