/***
 * 该类主要负责主窗口交互动作 和操作逻辑
 *
 * */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>

/**



menu
    run download viewHex compile edit dump hexdump rename remove


  */

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    refreshSerialPortTimer = new QTimer(this);
    //reConnectserialPortTimer = new QTimer(this);
    //parseSerialPortDataTimer = new QTimer(this);
    serialPortIdleInterruptTimer= new QTimer(this);

    serialPort = new QSerialPort();


    connect(refreshSerialPortTimer, &QTimer::timeout, this, &MainWindow::refreshSerialPort);
    //connect(reConnectserialPortTimer, &QTimer::timeout, this, &MainWindow::reConnectserialPort);
    //connect(parseSerialPortDataTimer, &QTimer::timeout, this, &MainWindow::parseSerialPortData);
    connect(serialPortIdleInterruptTimer, &QTimer::timeout, this, &MainWindow::serialPortIdleInterrupt);

    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::serialPortRecvDataCallback);
    connect(serialPort, &QSerialPort::errorOccurred, this, &MainWindow::serialOnBreak);

    ui->setupUi(this);

    ui->serialPortRecvTextBox->setWordWrapMode(QTextOption::NoWrap);
    ui->serialPortRecvTextBox->setLineWrapMode(QPlainTextEdit::NoWrap);

    refreshSerialPortTimer->setInterval(1000);
    refreshSerialPortTimer->start();

    //reConnectserialPortTimer->setInterval(500);
    //parseSerialPortDataTimer->setInterval(100);
    serialPortIdleInterruptTimer->setInterval(50);//空闲时间,自由设置
    // 列出一次串口列表
  //  refreshSerialPort();


    signaltoNoiseRatio.show();
    signaltoNoiseRatio.setWindowFlags(Qt::WindowStaysOnTopHint);
    locateInformation.show();
    locateInformation.setWindowFlags(Qt::WindowStaysOnTopHint);
    gpsModuleConfig.show();
    gpsModuleConfig.setWindowFlags(Qt::WindowStaysOnTopHint);

    //把串口指针直接传递给设置模块
    gpsModuleConfig.serialPort = this->serialPort;


    fileListViewModel = new QStringListModel(this);
    ui->fileListView->setModel(fileListViewModel);


}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::log(QString logInfo)
{

    ui->logTxtBox->appendPlainText(tool.getCurrentDateTime()  + logInfo);
}


/**
 * @brief MainWindow::refreshSerialPort refresh serial port 自动刷新串口列表
 */
void MainWindow::refreshSerialPort()
{

    // 获取最新的串口列表
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    //串口列表没有发生更新
    if (lastPortList.size() == ports.size()) {
        qDebug()<<"端口数量一致,跳过更新";
        //auto open  检查一下端口状态如果非人为关闭了,需要打开
        if(userPortAction && !serialPort->isOpen()) {
            openSerialPort();
        }

        return;
    }

    qDebug()<<"端口发现更新" << lastPortList.size()<<"<>" <<ports.size();

    recordLastPort= false;
    // 清空QComboBox的内容 注意会触发lastPort的更新
    ui->serialPortComboBox->clear();

    // 将串口列表添加到QComboBox中
    for (const QSerialPortInfo &portInfo : ports) {
       ui->serialPortComboBox->addItem(portInfo.portName());
    }
    lastPortList = ports;
    recordLastPort = true;
     //最后一次选择的端口 选中这个默认
    if ("" != lastSelectedQSerialPort) {
        int index = ui->serialPortComboBox->findText(lastSelectedQSerialPort);
        qDebug()<<"lastSelectedQSerialPort 查询结果"<<index;
        if (index != -1) {
            ui->serialPortComboBox->setCurrentIndex(index);
            //如果用户手动打开,未关闭,则自动打开
            if(userPortAction && !serialPort->isOpen()) {
                openSerialPort();
            }
        }
    }

}


//串口空闲中断
void MainWindow::serialPortIdleInterrupt() {
    //qDebug()<<"serialPortIdleInterruptserialPortIdleInterruptserialPortIdleInterruptserialPortIdleInterruptserialPortIdleInterrupt"<<endl;
    serialPortIdleInterruptTimer->stop();//请掉,避免反复触发
    parseSerialPortData() ;//解析
}




void MainWindow::listFileAction(const QString &recvData)
{
    // 提取完整的字符串
    QString completeData = recvData;



    QStringList fileList;
    int bufferSize=  bufferList.size() -1 ;
     qDebug()<<"发现listFileAction了:bufferSize"<<bufferSize<<endl;
        // 从当前行往前搜索文件特征行
    for (int i = bufferSize; i >0 ;  --i) {
        QString currentLine = bufferList.at(i);
        qDebug()<<"currentLine "<<currentLine<<endl;
        if ( currentLine.lastIndexOf("~~~File list START~~~") != -1) {

            qDebug()<<"发现start标记在 "<<i<<endl;
            for (int j = i;  j <  bufferSize;  ++j) {
                currentLine = bufferList.at(j);
                // 如果当前行符合文件特征，则添加到 fileList 中
                if (currentLine.contains(":") && currentLine.contains("bytes")) {
                    fileList << currentLine.trimmed();
                }
            }
        }

    }

    // 将 fileList 添加到 fileListViewModel 中
    fileListViewModel->setStringList(fileList);
}


/**

  把所有串口中缓存中的代码,格式化成一条条的QString数组
 * @brief MainWindow::formatSerialPortData
 * @return
 */

void MainWindow::parseSerialPortData() {

    int tmpCheckIndex = -1;
    // 清掉上次数据
    int endIndex = recvBuffer.indexOf("\r\n");
    while (endIndex != -1)
    {
        // 提取完整的字符串
       QString completeData = recvBuffer.left(endIndex);
       if (completeData.lastIndexOf("~~~File list END~~~") != -1) {
           listFileAction(completeData);
       }

       bufferList.append(completeData);
       // 删除已提取的数据及结束标记
       recvBuffer.remove(0, endIndex + 2);
       endIndex = recvBuffer.indexOf("\r\n");
        //增加时间戳
       if (addTimePrefix) {
           completeData = "["+tool.getCurrentDateTime()+"]"+completeData;
       }

       serialPortRecvTextBoxLastSliderPostion =  ui->serialPortRecvTextBox->verticalScrollBar()->sliderPosition();
       ui->serialPortRecvTextBox->appendPlainText(completeData);
       //收到数据, 绿框提示用户 以免同文本假死不知
       ui->serialPortRecvTextBox->setStyleSheet("border: 5px solid green");
       QTimer::singleShot(100, this, [this]() {
           QTimer::singleShot(100, this, [this]() {
               // 在这里添加你想要执行的代码，比如设置绿色边框
               ui->serialPortRecvTextBox->setStyleSheet("border: 3px solid green");
               QTimer::singleShot(100, this, [this]() {
                   // 在这里添加你想要执行的代码，比如设置绿色边框
                   ui->serialPortRecvTextBox->setStyleSheet("border: 1px solid green");
                   QTimer::singleShot(100, this, [this]() {
                       // 在这里添加你想要执行的代码，比如设置绿色边框
                       ui->serialPortRecvTextBox->setStyleSheet("border: 0px");
                   });
               });
           });
       });


        //自动滚动
       if (ui->authScrollCheckBox->isChecked()) {
          ui->serialPortRecvTextBox->moveCursor(QTextCursor::End);
       }
       //记录buffer大小
       bufferSize+=completeData.size();
       ui->recvBufferSizeLabel->setText(QString::number(bufferSize));
       //解析最新一条和据
    }





}
QString emptyStringToJsEmptyString(QString old) {
    if (old == "") return "\"\"";
    return "\""+old+"\"";
}
void  MainWindow::refreshViewLocate() {
    QString signalRunJsString =
            R"(
               updateInfo({latitude}, {longitude}, {altitude}, {utcDate}, {utcTime}, {speed}, {isAlready})
            )";
    RMC rmc = gnssParser.getGNSSRuntimeData().rmc;
    QList<int> seriesData;
    QList<QString> xAxisData;

    //直接替换模版
    signalRunJsString = signalRunJsString.replace("{isAlready}", gnssParser.isReady() ? "true" : "false" );
    signalRunJsString = signalRunJsString.replace("{latitude}", emptyStringToJsEmptyString(rmc.latitude)   );
    signalRunJsString = signalRunJsString.replace("{longitude}", emptyStringToJsEmptyString(rmc.longitude) );
    signalRunJsString = signalRunJsString.replace("{utcDate}", emptyStringToJsEmptyString(rmc.utcDate ));
    signalRunJsString = signalRunJsString.replace("{utcTime}", emptyStringToJsEmptyString(rmc.utcTime ));
    signalRunJsString = signalRunJsString.replace("{speed}", emptyStringToJsEmptyString(rmc.groundSpeed ));
    signalRunJsString = signalRunJsString.replace("{altitude}", emptyStringToJsEmptyString(gnssParser.getGNSSRuntimeData().gga.altitude));

    qDebug()<<"RunJS:"<<signalRunJsString<<endl;
    // log("RunJS" + signalRunJsString);
    locateInformation.webView->page()->runJavaScript(signalRunJsString);


}
void  MainWindow::refreshViewGSV() {
    //更新信噪比 生成触发命令
    QString signalRunJsString =
            R"(
            seriesData = [${seriesData}];
            xAxisData = [${xAxisData}];
            isAlready = [${isAlready}];
            viewTotal = ${viewTotal};
            usedTotal = ${usedTotal};
            setChatValue(seriesData, xAxisData, isAlready, viewTotal,usedTotal)

                                 )";
    GSV gsv = gnssParser.getGNSSRuntimeData().gsv;
    QString seriesDataString = "";
    QString xAxisDataString = "";
    QString readyListString = "";


    //计算出所有卫星编号跟所属类型
    for (const auto& key :  gsv.satellites.keys()) {
        SatelliteInfo sateLliteInfo = gsv.satellites.value(key);
        seriesDataString += QString::number(sateLliteInfo.snr) + ",";
        xAxisDataString += "\"" + gnssParser.getSatelliteType(sateLliteInfo.prn) + "\",";
        readyListString += (sateLliteInfo.isValid  ? "true" : "false") ;
        readyListString += ",";

    }
    //去掉拼装的最后一个,
    seriesDataString.chop(1);
    xAxisDataString.chop(1);
    readyListString.chop(1);


    //模版替换格式化
    signalRunJsString = signalRunJsString.replace("${usedTotal}",QString::number(gsv.totalUsed));
    signalRunJsString = signalRunJsString.replace("${viewTotal}",QString::number(gsv.totalSatellites));
    signalRunJsString = signalRunJsString.replace("${seriesData}",seriesDataString);
    signalRunJsString = signalRunJsString.replace("${xAxisData}",xAxisDataString);
    signalRunJsString = signalRunJsString.replace("${isAlready}",readyListString);
    //qDebug()<<"RunJS:"<<signalRunJsString<<endl;

    // log("RunJS" + signalRunJsString);

    signaltoNoiseRatio.webView->page()->runJavaScript(signalRunJsString);
}
void MainWindow::serialPortRecvData(QString cmd) {


    qDebug()<<"SerialPort RecvData : "<<cmd;
    recvBuffer.append(cmd);
    //空闲中断计时器开始
    serialPortIdleInterruptTimer->stop();
    serialPortIdleInterruptTimer->start();
}
void MainWindow::serialPortRecvDataCallback() {
    QByteArray data = serialPort->readAll();
    serialPortRecvData(data);
   //qDebug() << "recvBuffer len  " << recvBuffer.size() <<recvBuffer;
   //qDebug() << "bufferList len " << bufferList.size();


}
void MainWindow::on_commandLinkButton_clicked()
{
    refreshSerialPort();
}





void MainWindow::serialOnBreak(QSerialPort::SerialPortError error)
{
    qDebug() << "serialOnBreak  " << error;



    if (error != QSerialPort::NoError) {
       if (serialPort->isOpen()) {
           serialPort->close();
       }
       ui->openSerialPortButton->setText("打开串口");
    }

    if (error == QSerialPort::ResourceError || error == QSerialPort::DeviceNotFoundError || error == QSerialPort::PermissionError) {

       qDebug() << "ResourceError DeviceNotFoundError PermissionError   !,,,,,now reConnect";
    }





}

void MainWindow::sendCommand(QString cmd ) {


    if (serialPort && serialPort->isOpen()) {
        serialPort->write(cmd.toUtf8() + "\r\n"); // Send the command over the serial port
        serialPort->waitForBytesWritten(); // Wait for the write operation to complete
        qDebug()<<"成功写入串口";
    } else {
        qDebug()<<"串口未就绪";
    }

}
void MainWindow::openSerialPort(){

    // 设置串口名称和参数
    serialPort->setPortName(ui->serialPortComboBox->currentText());
    // serialPort->setBaudRate(QSerialPort::Baud9600);
    // serialPort->setDataBits(QSerialPort::Data8);
    // serialPort->setParity(QSerialPort::NoParity);
    // serialPort->setStopBits(QSerialPort::OneStop);


    // 从控件中读取串口参数并设置到 QSerialPort
    serialPort->setBaudRate(static_cast<QSerialPort::BaudRate>(ui->baudRateComboBox->currentText().toInt()));
    serialPort->setDataBits(static_cast<QSerialPort::DataBits>(ui->dataBitsComboBox->currentText().toInt()));
    serialPort->setParity(static_cast<QSerialPort::Parity>(ui->parityComboBox->currentText().toInt()));
    serialPort->setStopBits(static_cast<QSerialPort::StopBits>(ui->stopBitsComboBox->currentText().toInt()));

    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    // 打开串口
    if(!serialPort->open(QIODevice::ReadWrite)) {
        qDebug() <<ui->serialPortComboBox->currentText() << "Failed to open serial port.";
        return;
    }
    log("串口打开成功:" + ui->serialPortComboBox->currentText() + ",  "+ ui->baudRateComboBox->currentText());
    qDebug() << ui->serialPortComboBox->currentText() << "Serial port opened successfully.";

    userPortAction = true;
    ui->openSerialPortButton->setText("关闭串口");
}
void MainWindow::on_openSerialPortButton_clicked()
{
    if (ui->openSerialPortButton->text() == "打开串口") {
        openSerialPort();
    } else {
      log("已关闭串口 ");
       qDebug() << "Port status " <<serialPort->isOpen() << "Serial port opened successfully.";
       serialPort->close();
       userPortAction = false;
       ui->openSerialPortButton->setText("打开串口");

    }
}


void MainWindow::on_showSNR_stateChanged(int arg1)
{
    if (0 == arg1) {
        signaltoNoiseRatio.close();
    } else {
        signaltoNoiseRatio.webView->reload();
        signaltoNoiseRatio.show();
    }
}

void MainWindow::on_showLocate_stateChanged(int arg1)
{
    if (0 == arg1) {
        locateInformation.close();
    } else {
        locateInformation.webView->reload();
        locateInformation.show();
    }
}

void MainWindow::on_clearRecvDataButton_clicked()
{
    bufferSize = 0;
    bufferList.clear();
    ui->serialPortRecvTextBox->clear();
}

void MainWindow::on_saveRecvToFileButton_clicked()
{

    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss_zzz");
    QString fileName = currentDateTime + ".txt";
    QString filePath = QDir::currentPath() + "/" + fileName;
    // Display the file dialog
    // QString filePath = QFileDialog::getSaveFileName(nullptr, "保存文件", "", "GNSS调试数据 (*.txt)");
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        qDebug() << "保存文件:" << filePath << endl;
        // Open the file
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            // Write content to the file
            QTextStream stream(&file);
            stream << ui->serialPortRecvTextBox->toPlainText();
            file.close();
            log(filePath+"文件已保存");

            if (ui->clearAfterSavedCheckBox->isChecked()) {
                on_clearRecvDataButton_clicked();
            }

        } else {
            // File open failed
            log(filePath+"打开文件失败,无法保存!") ;
            // Handle the error
        }
    }
}

void MainWindow::on_configPanelCheckBox_stateChanged(int arg1)
{
    if (0 == arg1) {
        gpsModuleConfig.close();
    } else {
        gpsModuleConfig.show();
    }
}


void MainWindow::on_serialPortComboBox_currentTextChanged(const QString &arg1)
{
    if (recordLastPort) {
        qDebug()<<"lastSelectedQSerialPort : "<<arg1;
        lastSelectedQSerialPort = arg1;
        //用户已经打开而且用户手动切换时,自动打开
        if (userPortAction) {
            on_openSerialPortButton_clicked();
            on_openSerialPortButton_clicked();
        }
    }
}


void MainWindow::on_addTimePrefixcheckBox_stateChanged(int arg1)
{
    addTimePrefix = arg1 > 0;
}


void MainWindow::on_baudRateComboBox_currentIndexChanged(int index)
{
    if (recordLastPort && userPortAction) {
        //用户已经打开而且用户手动切换时,自动切换波特率
            on_openSerialPortButton_clicked();
            on_openSerialPortButton_clicked();
    }
}


void MainWindow::on_simulatorCheckbox_stateChanged(int arg1)
{
    if (arg1 > 0) {
        auto callbackFunction = [this](const QString &data) {
            this->serialPortRecvData(data);
            //qDebug() << "Received GNSS data: " << data;
        };
        gnssSimulator.setCallbackFunction(callbackFunction);
        gnssSimulator.start();
    } else {
        gnssSimulator.stop();

    }
}


void MainWindow::on_showFIleList_clicked()
{

    // 清空模型数据
    fileListViewModel->setStringList(QStringList());
    //以下代码借鉴了esplorer的操作日志感谢作者
    QString cmd = R"(
                   _dir=function()
                      local k,v,l
                      print("~~~File ".."list START~~~")
                      for k,v in pairs(file.list()) do
                          l = string.format("%-15s",k)
                          print(l.." : "..v.." bytes")
                      end
                      print("~~~File ".."list END~~~")
                    end;
                  _dir();
                  _dir=nil


              )";
    //sendCommand(cmd);

    // 添加新项
//    QStringList newList;
//    newList << "Item 1" << "Item 2" << "Item 3";
//    fileListViewModel->setStringList(newList);



    cmd = R"(
          _download = function(p)local f = io.open(p, "rb") if f then  print("__download__file__start__"  )
                  local c,hex_content
                  while true do
                      hex_content = "";
                      c = f:read(1024)if not c then break;end;
                      for i = 1, #c do
                          hex_content = hex_content .. string.format("%02X", string.byte(c, i))
                      end
                      print("__download__line_start__" .. hex_content .. "__download__line_end__")tmr.wdclr()
                  end
                  f:close() print("__download__file__end__"  )
              end
          end
          _download("system.lc")
          _download = nil;
          )";
                      sendCommand(cmd);

}


