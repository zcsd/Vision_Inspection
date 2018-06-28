#include "settingdialog.h"
#include "ui_settingdialog.h"

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
    initSetup();
}

void SettingDialog::initSetup()
{
    readConf();
    QStringList resolutionList = {"2448x2048", "1224x1024", "2048x1536", "1024x768"};
    ui->comboBoxRes->addItems(resolutionList);
    QString fpsValue = "35";
    ui->lineEditFPS->setText(fpsValue);
    QStringList expoModeList = {"Off", "On", "Auto"};
    ui->comboBoxExpMode->addItems(expoModeList);
    QString expoTimeValue = "20000"; // us
    ui->lineEditExpTime->setText(expoTimeValue);
    QStringList wbModeList = {"Off", "Auto"};
    ui->comboBoxWB->addItems(wbModeList);
    QStringList gainModeList = {"Off", "Auto"};
    ui->comboBoxGainMode->addItems(gainModeList);
    QString gainValue = "4.0";
    ui->lineEditGainValue->setText(gainValue);
    QStringList pixelFormatList = {"RGB", "BGR", "YCbCr", "Mono"};
    ui->comboBoxPF->addItems(pixelFormatList);
    bool centerX = false, centerY = false;
    ui->checkBoxCenterX->setChecked(centerX);
    ui->checkBoxCenterY->setChecked(centerY);
    QString offsetXValue = "8";
    QString offsetYValue = "4";
    ui->lineEditOXValue->setText(offsetXValue);
    ui->lineEditOYValue->setText(offsetYValue);
    QString bufferSize = "1";
    ui->lineEditBufferSize->setText(bufferSize);
}

void SettingDialog::readConf()
{
   QStringList strListCameraParaName = {"ExposureAuto\t", "ExposureMode\t", "ExposureTime\t", "BalanceWhiteAuto\t",
                                        "GainAuto\t", "Gain\t", "Width\t", "Height\t", "OffsetX\t", "OffsetY\t",
                                        "CenterX\t", "CenterY\t", "PixelFormat\t", "AcquisitionFrameRate\t"};

   QStringList strListCameraParaValue = {};
   QFile confFile("../conf/acA2040-55uc_22095198.pfs");
   confFile.open(QIODevice::ReadOnly);

   QTextStream in (&confFile);
   QString line;
   int counterFound = 0;
   do {
       line = in.readLine();
       for (int i = 0; i < strListCameraParaName.size(); i++) {
           if (line.contains(strListCameraParaName[i], Qt::CaseSensitive)) {
               line.remove(0, strListCameraParaName[i].length());
               strListCameraParaValue.append(line);
               counterFound++;
           }
       }
       if (counterFound == strListCameraParaName.size())
           break;
   } while (!line.isNull());
   qDebug() << strListCameraParaValue;
}

SettingDialog::~SettingDialog()
{
    delete ui;
}
