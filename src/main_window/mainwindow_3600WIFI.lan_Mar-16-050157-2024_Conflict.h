#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QScrollBar>

#include "signaltonoiseratio.h"
#include "locateinformation.h"
#include "gnssparser.h"
#include "tool.h"
#include "gpsmoduleconfig.h"
#include "gnsssimulator.h"
#if defined(Q_OS_WIN) // Windows
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#elif defined(Q_OS_MAC) // macOS
#include <QtSerialPort>
#endif


/**

  第一次使用QT写功能性程序, 逻辑安排,目录结构等较欠缺,望海涵

 * @brief The GPSInfo struct
 */



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void refreshSerialPort();
    void log(QString logInfo);
    void on_commandLinkButton_clicked();
    void serialPortRecvDataCallback();
    void serialOnBreak(QSerialPort::SerialPortError error);
    void on_openSerialPortButton_clicked();
    void on_showSNR_stateChanged(int arg1);
    void on_showLocate_stateChanged(int arg1);
    void parseSerialPortData();

    void on_clearRecvDataButton_clicked();
    //渲染相关的
    void   refreshViewGSV();
    void   refreshViewLocate();

    void on_saveRecvToFileButton_clicked();
    void serialPortIdleInterrupt();

    void on_configPanelCheckBox_stateChanged(int arg1);

    void on_serialPortComboBox_currentTextChanged(const QString &arg1);
    void openSerialPort();
    void on_addTimePrefixcheckBox_stateChanged(int arg1);

    void on_baudRateComboBox_currentIndexChanged(int index);

    void on_simulatorCheckbox_stateChanged(int arg1);
    void serialPortRecvData(QString cmd);
    void  listFileAction(const QString &recvData);

    void on_showFIleList_clicked();
    void sendCommand(QString cmd );

private:
    bool addTimePrefix = false;
    Ui::MainWindow *ui;
    QTimer *refreshSerialPortTimer ;
    QTimer *reConnectserialPortTimer ;
   // QTimer *parseSerialPortDataTimer ;
    QTimer *serialPortIdleInterruptTimer ;//串口中断计时器


    QSerialPort *serialPort;
    long serialPortRecvTextBoxLastSliderPostion;
    QString recvBuffer;
    QList<QString> bufferList;
    SignaltoNoiseRatio signaltoNoiseRatio;
    LocateInformation locateInformation;
    long bufferSize = 0;
    GNSSParser gnssParser;;
    Tool tool;
    GpsModuleConfig gpsModuleConfig;

    QList<QSerialPortInfo> lastPortList;//最次检测到的端口列表
    QString   lastSelectedQSerialPort;//最后一次选中的端口
    bool recordLastPort = true;//是否记录最后端口. 比如clear和添加过程时,不能去记录最后光标.只有用户手动操作时才记录
    bool userPortAction = false;//用户是否是手动状态, 用于不小心串口断开时自动连接

    GnssSimulator gnssSimulator;

    // 创建字符串列表模型
    QStringListModel *fileListViewModel;

};
#endif // MAINWINDOW_H
