/**
 *对QT动态组件还不熟,时间关系先堆积代码吧.
 *等熟悉后再进行调整
 *从0到1先实现功能
 *
 * */


#include "gpsmoduleconfig.h"
#include "ui_gpsmoduleconfig.h"
#include <QDebug>
#include <QClipboard>


GpsModuleConfig::GpsModuleConfig(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GpsModuleConfig)
{

    ui->setupUi(this);
}

GpsModuleConfig::~GpsModuleConfig()
{
    delete ui;
}

void GpsModuleConfig::on_GpsModuleConfig_finished(int result)
{


}
QString GpsModuleConfig::sendCommand(QString cmd ) {

    ui->lastCmdText->setText(cmd);

    if (serialPort && serialPort->isOpen()) {
        serialPort->write(cmd.toUtf8() + "\r\n"); // Send the command over the serial port
        serialPort->waitForBytesWritten(); // Wait for the write operation to complete
        qDebug()<<"成功写入串口";
    } else {
        qDebug()<<"串口未就绪";
    }

    return cmd;

}
QString GpsModuleConfig::getCommandString(QString cmd, QString value)
{
    QString command =  cmd + "," + value;
    if (value == "") {
        command = cmd;
    }
    //如 "$PCAS05,2*1A\r\n"
    //把命令和值用,连起来(PCAS05,2). 把所有的值进行异或.得到了1A 然后在首尾拼上$和*校验结果. 就是命令了
    // 计算异或校验值
    char checksum = 0;
    for (int i = 0; i < command.size(); ++i) {
        checksum = checksum ^ command.at(i).toLatin1();
    }

    // 构造最终的命令字符串
    QString finalCommand = "$" + command + "*";
    QString checksumStr = QString("%1").arg(checksum, 2, 16, QChar('0')).toUpper();
    finalCommand += checksumStr + "\r\n";
    return finalCommand;
}



void GpsModuleConfig::on_baudrate_btn_4800_clicked()
{

    qDebug()<<sendCommand(getCommandString("PCAS01", "0"));

}



void GpsModuleConfig::on_baudrate_btn_9600_clicked()
{
 qDebug()<<sendCommand(getCommandString("PCAS01", "1"));
}


void GpsModuleConfig::on_baudrate_btn_19200_clicked()
{
 qDebug()<<sendCommand(getCommandString("PCAS01", "2"));
}


void GpsModuleConfig::on_baudrate_btn_38400_clicked()
{
 qDebug()<<sendCommand(getCommandString("PCAS01", "3"));
}


void GpsModuleConfig::on_baudrate_btn_57600_clicked()
{
 qDebug()<<sendCommand(getCommandString("PCAS01", "4"));
}


void GpsModuleConfig::on_baudrate_btn_115200_clicked()
{
 qDebug()<<sendCommand(getCommandString("PCAS01", "5"));
}


void GpsModuleConfig::on_baudrate_btn_230400_clicked()
{
 qDebug()<<sendCommand(getCommandString("PCAS01", "6"));
}


void GpsModuleConfig::on_baudrate_btn_460800_clicked()
{
     qDebug()<<sendCommand(getCommandString("PCAS01", "7"));
}


void GpsModuleConfig::on_response_1_hz_clicked()
{
    qDebug()<<sendCommand(getCommandString("PCAS02", "1000"));
}


void GpsModuleConfig::on_response_2_hz_clicked()
{
qDebug()<<sendCommand(getCommandString("PCAS02", "500"));
}


void GpsModuleConfig::on_response_4_hz_clicked()
{
qDebug()<<sendCommand(getCommandString("PCAS02", "250"));
}


void GpsModuleConfig::on_response_5_hz_clicked()
{
qDebug()<<sendCommand(getCommandString("PCAS02", "200"));
}


void GpsModuleConfig::on_response_10_hz_clicked()
{
qDebug()<<sendCommand(getCommandString("PCAS02", "100"));
}


void GpsModuleConfig::on_response_20_hz_clicked()
{
qDebug()<<sendCommand(getCommandString("PCAS02", "50"));
}


void GpsModuleConfig::on_setNMEA_clicked()
{
    QString value = "";
    for(int i=0;i<8;i++) {
        value += ( nmeaCheckStatus[i] ? "1,": "0,");
    }

    qDebug()<<sendCommand(getCommandString("PCAS03", value.chopped(1)));
}


void GpsModuleConfig::on_ggaCheckBox_stateChanged(int arg1)
{
    nmeaCheckStatus[0] = arg1 >0;

}


void GpsModuleConfig::on_gllCheckBox_stateChanged(int arg1)
{
    nmeaCheckStatus[1] = arg1 >0;
}


void GpsModuleConfig::on_gsaCheckBox_stateChanged(int arg1)
{
    nmeaCheckStatus[2] = arg1 >0;
}


void GpsModuleConfig::on_gsvCheckBox_stateChanged(int arg1)
{
    nmeaCheckStatus[3] = arg1 >0;
}


void GpsModuleConfig::on_rmcCheckBox_stateChanged(int arg1)
{
    nmeaCheckStatus[4] = arg1 >0;
}


void GpsModuleConfig::on_vtgCheckBox_stateChanged(int arg1)
{
    nmeaCheckStatus[5] = arg1 >0;
}


void GpsModuleConfig::on_zdaCheckBox_stateChanged(int arg1)
{
    nmeaCheckStatus[6] = arg1 >0;
}


void GpsModuleConfig::on_txtCheckBox_stateChanged(int arg1)
{
    nmeaCheckStatus[7] = arg1 >0;
}


void GpsModuleConfig::on_gpsCheckBox_stateChanged(int arg1)
{
    satelliteCheckStatus[0] = arg1 >0;

}
void GpsModuleConfig::on_bdsCheckBox_stateChanged(int arg1)
{
    satelliteCheckStatus[1] = arg1 >0;
}

void GpsModuleConfig::on_gsaCheckBox_2_stateChanged(int arg1)
{
    satelliteCheckStatus[2] = arg1 >0;
}


void GpsModuleConfig::on_stlConfigButton_clicked()
{
    int result = 0;
    for (int i = 0 ; i<3;i++) {
        if (satelliteCheckStatus[i])result |= 1<<i;
    }
    if (result == 0) {
        result = 7;
    }
    qDebug()<<sendCommand(getCommandString("PCAS04", QString::number(result)));


}


void GpsModuleConfig::on_VersionRadioButton_2_clicked()
{
    versionSelect = 5;
}


void GpsModuleConfig::on_VersionRadioButton_clicked()
{
    versionSelect = 2;
}


void GpsModuleConfig::on_VersionRadioButton_3_clicked()
{
    versionSelect = 9;
}


void GpsModuleConfig::on_serVersionButton_clicked()
{
     qDebug()<<sendCommand(getCommandString("PCAS05", QString::number(versionSelect)));
}


void GpsModuleConfig::on_quueryInfopushButton_clicked()
{
    //版本号查询
    qDebug()<<sendCommand(getCommandString("PCAS06",  "0"));

}


void GpsModuleConfig::on_quueryInfopushButton_2_clicked()
{
    // 1=查询硬件型号及序列号
        qDebug()<<sendCommand(getCommandString("PCAS06",  "1"));
}


void GpsModuleConfig::on_quueryInfopushButton_3_clicked()
{
    // 2 查询多模接收机的工作模式
     qDebug()<<sendCommand(getCommandString("PCAS06",  "2"));
}


void GpsModuleConfig::on_quueryInfopushButton_4_clicked()
{
     // 3=查询产品的客户编号
     qDebug()<<sendCommand(getCommandString("PCAS06",  "3"));
}


void GpsModuleConfig::on_quueryInfopushButton_5_clicked()
{
    // 5 查询升级代码信息
     qDebug()<<sendCommand(getCommandString("PCAS06",  "5"));
}


void GpsModuleConfig::on_restartPushButton_clicked()
{
    //热启动
      qDebug()<<sendCommand(getCommandString("PCAS10",  "0"));
}


void GpsModuleConfig::on_restartPushButton_2_clicked()
{
    qDebug()<<sendCommand(getCommandString("PCAS10",  "1"));
}


void GpsModuleConfig::on_restartPushButton_3_clicked()
{
    qDebug()<<sendCommand(getCommandString("PCAS10",  "2"));
}


void GpsModuleConfig::on_restartPushButton_4_clicked()
{
    qDebug()<<sendCommand(getCommandString("PCAS10",  "3"));
}


void GpsModuleConfig::on_restartPushButton_5_clicked()
{
    qDebug()<<sendCommand(getCommandString("PCAS10",  "8"));
}


void GpsModuleConfig::on_restartPushButton_6_clicked()
{
    qDebug()<<sendCommand(getCommandString("PCAS10",  "9"));
}



void GpsModuleConfig::on_copyButton_clicked()
{
    // 获取剪贴板
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->lastCmdText->text());
}



void GpsModuleConfig::on_copyCButton_clicked()
{
    QString cmdString = ui->lastCmdText->text();
    cmdString.chop(2);//remote \r\n
    QString cCode = "char cmdbuf[] = \"" + cmdString + "\\r\\n\";";
    // 获取剪贴板
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(cCode);

}


void GpsModuleConfig::on_save_to_flash_btn_clicked()
{
       qDebug()<<sendCommand(getCommandString("PCAS00",  ""));

}

