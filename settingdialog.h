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

private:
    Ui::SettingDialog *ui;
    void initSetup();
    void readConf();
};

#endif // SETTINGDIALOG_H
