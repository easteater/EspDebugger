#ifndef GPSMODULECONFIG_H
#define GPSMODULECONFIG_H

#include <QDialog>
#include <QSerialPort>

namespace Ui {
class GpsModuleConfig;
}

class GpsModuleConfig : public QDialog
{
    Q_OBJECT

public:
    explicit GpsModuleConfig(QWidget *parent = nullptr);
     QSerialPort *serialPort;
    ~GpsModuleConfig();

private slots:
    void on_GpsModuleConfig_finished(int result);

    void on_baudrate_btn_4800_clicked();
    QString getCommandString(QString cmd,QString value);
    QString sendCommand(QString cmd );

    void on_baudrate_btn_9600_clicked();

    void on_baudrate_btn_19200_clicked();

    void on_baudrate_btn_38400_clicked();

    void on_baudrate_btn_57600_clicked();

    void on_baudrate_btn_115200_clicked();

    void on_baudrate_btn_230400_clicked();

    void on_baudrate_btn_460800_clicked();

    void on_response_1_hz_clicked();

    void on_response_2_hz_clicked();

    void on_response_4_hz_clicked();

    void on_response_5_hz_clicked();

    void on_response_10_hz_clicked();

    void on_response_20_hz_clicked();

    void on_setNMEA_clicked();

    void on_ggaCheckBox_stateChanged(int arg1);

    void on_gllCheckBox_stateChanged(int arg1);

    void on_gsaCheckBox_stateChanged(int arg1);

    void on_gsvCheckBox_stateChanged(int arg1);

    void on_rmcCheckBox_stateChanged(int arg1);

    void on_vtgCheckBox_stateChanged(int arg1);

    void on_zdaCheckBox_stateChanged(int arg1);

    void on_txtCheckBox_stateChanged(int arg1);

    void on_bdsCheckBox_stateChanged(int arg1);

    void on_gpsCheckBox_stateChanged(int arg1);

    void on_gsaCheckBox_2_stateChanged(int arg1);

    void on_stlConfigButton_clicked();

    void on_VersionRadioButton_2_clicked();

    void on_VersionRadioButton_clicked();

    void on_VersionRadioButton_3_clicked();

    void on_serVersionButton_clicked();

    void on_quueryInfopushButton_clicked();

    void on_quueryInfopushButton_2_clicked();

    void on_quueryInfopushButton_3_clicked();

    void on_quueryInfopushButton_4_clicked();

    void on_quueryInfopushButton_5_clicked();

    void on_restartPushButton_clicked();

    void on_restartPushButton_2_clicked();

    void on_restartPushButton_3_clicked();

    void on_restartPushButton_4_clicked();

    void on_restartPushButton_5_clicked();

    void on_restartPushButton_6_clicked();

    void on_copyButton_clicked();

    void on_copyCButton_clicked();

    void on_save_to_flash_btn_clicked();

private:
    Ui::GpsModuleConfig *ui;
    bool   nmeaCheckStatus[8] = {true,true,true,true,true,true,true,true};
    bool   satelliteCheckStatus[3] = {true,true,true};
    int   versionSelect = 5;
};

#endif // GPSMODULECONFIG_H
