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
    cameraParaMap.clear();
    readConf();

    QStringList resolutionList = {"2448x2048", "1224x1024", "2048x1536", "1024x768"};
    ui->comboBoxRes->addItems(resolutionList);
    ui->comboBoxRes->setCurrentText(cameraParaMap["Width"] + "x" + cameraParaMap["Height"]);

    ui->lineEditFPS->setText(cameraParaMap["AcquisitionFrameRate"]);

    QStringList expoModeList = {"Off", "On", "Auto"};
    ui->comboBoxExpMode->addItems(expoModeList);
    if (cameraParaMap["ExposureAuto"] == "Off" && cameraParaMap["ExposureMode"] == "Timed") {
        ui->comboBoxExpMode->setCurrentText("Off");
        ui->lineEditExpTime->setText(cameraParaMap["ExposureTime"]);
    } else if (cameraParaMap["ExposureAuto"] == "On") {
        ui->comboBoxExpMode->setCurrentText("Auto");
        ui->lineEditExpTime->setEnabled(false);
    }

    QStringList wbModeList = {"Off", "On"};
    ui->comboBoxWB->addItems(wbModeList);
    ui->comboBoxWB->setCurrentText(cameraParaMap["BalanceWhiteAuto"]);

    QStringList gainModeList = {"Off", "On"};
    ui->comboBoxGainMode->addItems(gainModeList);
    if (cameraParaMap["GainAuto"] == "Off") {
        ui->comboBoxGainMode->setCurrentText("Off");
        ui->lineEditGainValue->setText(cameraParaMap["Gain"]);
    } else if (cameraParaMap["GainAuto"] == "On") {
        ui->comboBoxGainMode->setCurrentText("On");
        ui->lineEditGainValue->setEnabled(false);
    }

    QStringList pixelFormatList = {"Bayer RG8", "RGB", "BGR", "YCbCr", "Mono 8"};
    ui->comboBoxPF->addItems(pixelFormatList);
    ui->comboBoxPF->setCurrentText(cameraParaMap["PixelFormat"]);

    if (cameraParaMap["CenterX"] == "0")
        ui->checkBoxCenterX->setChecked(false);
    else
        ui->checkBoxCenterX->setChecked(true);

    if (cameraParaMap["CenterY"] == "0")
        ui->checkBoxCenterY->setChecked(false);
    else
        ui->checkBoxCenterY->setChecked(true);

    ui->lineEditOXValue->setText(cameraParaMap["OffsetX"]);
    ui->lineEditOYValue->setText(cameraParaMap["OffsetY"]);
    // TODO
    ui->lineEditBufferSize->setText("5");
}

void SettingDialog::readConf()
{
   QStringList strListCameraParaName = {"ExposureAuto\t", "ExposureMode\t", "ExposureTime\t", "BalanceWhiteAuto\t",
                                        "GainAuto\t", "Gain\t", "Width\t", "Height\t", "OffsetX\t", "OffsetY\t",
                                        "CenterX\t", "CenterY\t", "PixelFormat\t", "AcquisitionFrameRate\t"};

   //QFile confFile("../conf/acA2040-55uc_22095198.pfs");
   QFile confFile("../conf/acA2440-35uc_22776933.pfs");
   confFile.open(QIODevice::ReadOnly);

   QTextStream in (&confFile);
   QString line, tempStr;
   int counterFound = 0;

   do {
       line = in.readLine();
       for (int i = 0; i < strListCameraParaName.size(); i++) {
           if (line.contains(strListCameraParaName[i], Qt::CaseSensitive)) {
               line.remove(0, strListCameraParaName[i].length());
               tempStr = strListCameraParaName[i];
               cameraParaMap[tempStr.remove(tempStr.length()-1, tempStr.length())] = line;
               counterFound++;
           }
       }
       if (counterFound == strListCameraParaName.size())
           break;
   } while (!line.isNull());
   confFile.close();
}

void SettingDialog::updateConf()
{
    QStringList strListCameraParaName = {"ExposureTime\t","AcquisitionFrameRate\t"};
  /*  QStringList strListCameraParaName = {"ExposureAuto\t", "ExposureMode\t", "ExposureTime\t", "BalanceWhiteAuto\t",
                                         "GainAuto\t", "Gain\t", "Width\t", "Height\t", "OffsetX\t", "OffsetY\t",
                                         "CenterX\t", "CenterY\t", "PixelFormat\t", "AcquisitionFrameRate\t"};*/
    //QFile confFile("../conf/acA2040-55uc_22095198.pfs");
    QFile confFile("../conf/acA2440-35uc_22776933.pfs");
    confFile.open(QIODevice::ReadOnly);
    QFile newConfFile("../conf/temp.pfs");
    newConfFile.open(QIODevice::WriteOnly|QIODevice::Truncate);
    QTextStream out (&newConfFile);

    QTextStream in (&confFile);
    QString line, tempStr;
    bool found = false, finished = false;
    int counterFound = 0;
    do {
        line = in.readLine();
        found = false;

        if (!finished) {
            for (int i = 0; i < strListCameraParaName.size(); i++) {
                if (line.contains(strListCameraParaName[i], Qt::CaseSensitive)) {
                    out << ( strListCameraParaName[i] + getParameters(strListCameraParaName[i]) + "\n" );
                    counterFound++;
                    found = true;
                    continue;
                }
            }
        }

        if (found == false) {
            out << line;
            if (!in.atEnd())
                out << "\n";
        }

        if (counterFound == strListCameraParaName.size())
            finished = true;

    } while (!line.isNull());
    confFile.close();
    newConfFile.close();
    confFile.remove();
    //newConfFile.rename("../conf/temp.pfs", "../conf/acA2040-55uc_22095198.pfs");
    newConfFile.rename("../conf/temp.pfs", "../conf/acA2440-35uc_22776933.pfs");
}

QString SettingDialog::getParameters(QString nameParameter)
{
    if (nameParameter == "ExposureTime\t")
        return ui->lineEditExpTime->text();
    else if (nameParameter == "AcquisitionFrameRate\t")
        return ui->lineEditFPS->text();
}

void SettingDialog::on_pushButtonCancel_clicked()
{
    this->done(Rejected);
}

void SettingDialog::on_pushButtonOK_clicked()
{
    updateConf();
    this->done(Accepted);
}

SettingDialog::~SettingDialog()
{
    delete ui;
}
