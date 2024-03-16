/***
 * 该类主要负责主窗口交互动作 和操作逻辑
 *
 * */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>



/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    refreshSerialPortTimer = new QTimer(this);
    serialPortIdleInterruptTimer= new QTimer(this);

    serialPort = new QSerialPort();


    connect(refreshSerialPortTimer, &QTimer::timeout, this, &MainWindow::refreshSerialPort);
    connect(serialPortIdleInterruptTimer, &QTimer::timeout, this, &MainWindow::serialPortIdleInterrupt);

    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::serialPortRecvDataCallback);
    connect(serialPort, &QSerialPort::errorOccurred, this, &MainWindow::serialOnBreak);

    ui->setupUi(this);

    ui->serialPortRecvTextBox->setWordWrapMode(QTextOption::NoWrap);
    ui->serialPortRecvTextBox->setLineWrapMode(QPlainTextEdit::NoWrap);

    refreshSerialPortTimer->setInterval(1000);
    refreshSerialPortTimer->start();

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



    //右键相关
   menuListCaption  <<
            "execute" <<"compile" <<"-"<< "download"<< "upload" <<"-"<< "viewHex" <<
            "dump"<< "-"   <<"rename"<<        "remove";


   for (const QString &caption : menuListCaption) {
       QString iconName = QString(":/resource/images/%1.png").arg(caption);
       QIcon icon(iconName);
       QIcon scaledIcon = icon.pixmap(QSize(128, 128));
       QString capitalizedCaption = caption;
       capitalizedCaption[0] = capitalizedCaption[0].toUpper();
       fileRightButtionAction[caption] = new QAction(scaledIcon, capitalizedCaption, this);
       fileRightButtonContextMenu.addAction(fileRightButtionAction[caption]);
       qDebug() << caption << fileRightButtionAction[caption] << endl;
   }


    //menu 命令
    fileRightButtionActionCmd["execute"] = R"(dofile("%1"))";
    fileRightButtionActionCmd["-"] = "";
    fileRightButtionActionCmd["upload"] = "";
    fileRightButtionActionCmd["download"] = QString(R"(
                                _download = function(p)print(p)
                                    local f = io.open(p, "rb")
                                    if f then
                                        print("__download__file__start__")
                                        local c,hex_content
                                        while true do
                                            hex_content = "";
                                            c = f:read(1024)
                                            if not c then
                                                break;
                                            end;
                                            for i = 1, #c do
                                                hex_content = hex_content .. string.format("%02X", string.byte(c, i))
                                            end
                                            print("__download__line_start__" .. hex_content .. "__download__line_end__")
                                            tmr.wdclr()
                                        end
                                        f:close()
                                        print("__download__file__end__")
                                    end
                                end
                                _download("%1")
                                _download = nil; )");
    fileRightButtionActionCmd["viewHex"] = QString(R"(
                                                   _download = function(p)print(p)
                                                       local f = io.open(p, "rb")
                                                       if f then
                                                           local c,hex_content,address=0,"",0
                                                           while true do
                                                               hex_content = "";
                                                               c = f:read(32)
                                                               if not c then
                                                                   break;
                                                               end;
                                                               for i = 1, #c do
                                                                   hex_content = hex_content .. string.format("%02X", string.byte(c, i))
                                                               end
                                                               print("0x" .. string.format("%08X",address) .. ":".. hex_content )
                                                               tmr.wdclr()
                                                                address = address + 32
                                                           end
                                                           f:close()
                                                       end
                                                   end
                                                   _download("%1")
                                                   _download = nil; )");
    fileRightButtionActionCmd["compile"] = R"(node.compile("%1"))";
    fileRightButtionActionCmd["dump"] = QString(R"(
                                                _download = function(p)print(p)
                                                    local f = io.open(p, "rb")
                                                    if f then
                                                        local c,hex_content,address=0,"",0
                                                        while true do
                                                            hex_content = "";
                                                            c = f:read(1024)
                                                            if not c then
                                                                break;
                                                            end;

                                                            print(c)
                                                            tmr.wdclr()
                                                             address = address + 32
                                                        end
                                                        f:close()
                                                    end
                                                end
                                                _download("%1")
                                                _download = nil; )");
    fileRightButtionActionCmd["rename"] = "";
    fileRightButtionActionCmd["remove"] = R"(file.remove("%1"))";






}
void MainWindow::uploadFile() {
    QString cmd = QString(R"(
        __w=function(f,h)
            local e=io.open(f,"wb")
            if e then
                for i=1,#h,2 do
                    e:write(string.char(tonumber(h:sub(i,i+1),16)))
                end
                e:close()
            end
        end
        __w("%1","%2")
    )");

    QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath());
    if (!filePath.isEmpty()) {
        // 读取文件内容并处理
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QString fileName = QFileInfo(filePath).fileName(); // 获取文件名
            QByteArray fileData = file.readAll();
             ui->fileProgressBar->setValue(0);
            //按32个byte进行遍历整个数据 并调用 send(data)
            int currentChunk = 0;
            int chunkSize = 32; // 设置每个数据块的大小为32字节
            int dataSize = fileData.size();
            int totalChunks = dataSize / chunkSize;
            for (int i = 0; i < dataSize; i += chunkSize) {
                currentChunk++;
                QByteArray chunk = fileData.mid(i, chunkSize); // 从文件数据中提取32字节的块
                QString hexChunk = QString(chunk.toHex());
                sendCommand(QString(cmd).arg(fileName, hexChunk) + "\r\n\r\n"); // 发送数据块到设备
               int progress = (static_cast<double>(currentChunk) / totalChunks) * 100;
               qDebug()<<"上传进度:"<<progress<<endl;
               qDebug()<<"执行命令:"<<QString(cmd).arg(fileName, hexChunk)<<endl;
               ui->fileProgressBar->setValue(progress);
               ui->fileProgressBar->update(); // 更新UI显示
            }
            //
            file.close();
             sendCommand("__w=nil\r\n\r\n"); // 发送数据块到设备
        } else {
            qDebug() << "Failed to open file for reading:" << file.errorString();
        }
    }
}


void MainWindow::on_fileListView_customContextMenuRequested(const QPoint &pos)
{

    QModelIndex index = ui->fileListView->indexAt(pos);
    // 获取文件名
    QString selectedText = index.data(Qt::DisplayRole).toString();
    QString fileName = selectedText.contains(":") ? selectedText.split(":").first().trimmed() : "??";
    // 检查文件是否是lua或lc文件
    bool isLuaFile = fileName.endsWith(".lua", Qt::CaseInsensitive);
    bool isLcFile = fileName.endsWith(".lc", Qt::CaseInsensitive);
    qDebug()<<selectedText<<fileName<<endl;
    fileRightButtonContextMenu.clear();
    // 向右键菜单中添加菜单项并设置启用状态
    for (const QString& actionName : menuListCaption) {
        QString actionNameUpper = actionName;
        actionNameUpper[0] = actionNameUpper[0].toUpper();
         QAction* action = fileRightButtionAction[actionName];
         if (!action) {
             continue;
         }
         if ( actionName == "-") {
            fileRightButtonContextMenu.addSeparator();
            continue;
         }
         if (actionName == "upload") {
             disconnect(action, &QAction::triggered, this, nullptr);
             connect(action, &QAction::triggered, this, [this]() {
                uploadFile();
                on_showFIleList_clicked();
             });
         }

         // 如果 filename 为 "??", 禁用此项
         if (fileName == "??" ) {
             action->setText(actionNameUpper);
             action->setEnabled(false);
         } else {
             QString caption = actionNameUpper +" " + (fileName);
             action->setText(caption); // 在 caption 中追加 filename
             action->setEnabled(true); // 启用此项
             // 连接动作与命令
             QString execCmd = QString( fileRightButtionActionCmd[actionName]).arg(fileName);
             if (execCmd == "") {
                 //重命名
                if (actionName == "rename") {
                    //弹出输入框,输入框默认值为选中的文件名 fileName
                    disconnect(action, &QAction::triggered, this, nullptr);
                    connect(action, &QAction::triggered, this, [this, actionName, execCmd,fileName]() {
                        QString newName = QInputDialog::getText(this, "Rename File", "Enter new name:", QLineEdit::Normal, fileName);

                        qDebug()<<"rename "<<QString(R"(file.rename("%1","%2"))").arg(fileName,newName)<<endl;
                        if (newName.isEmpty()) return;
                         sendCommand(QString(R"(file.rename("%1","%2"))").arg(fileName,newName));
                           on_showFIleList_clicked();
                    });
                }

             } else {
                 disconnect(action, &QAction::triggered, this, nullptr);
                 connect(action, &QAction::triggered, this, [this, actionName, execCmd]() {
                     qDebug()<<"触发命令"<<execCmd<<endl;
                     sendCommand(execCmd + "\r\n\r\n");
                     if ( actionName == "remove" || actionName == "compile"  ) {
                        on_showFIleList_clicked();
                     }
                 });
             }
         }
         // !lc !lua and execute item
         if (!isLuaFile && !isLcFile && actionName == "execute") {
             action->setEnabled(false);
         }
         // enable only and e lua
         if (!isLuaFile  && actionName == "compile") {
             action->setEnabled(false);
         }
         // enable only and e lua
         if (actionName == "upload") {
             action->setText(actionNameUpper);
             action->setEnabled(1);
         }
         QFont font = action->font();
            font.setPointSize(18);
            action->setFont(font);
         // 添加动作到菜单

         fileRightButtonContextMenu.addAction(action);
     }


    // 显示菜单
    fileRightButtonContextMenu.exec(ui->fileListView->mapToGlobal(pos));

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
        //qDebug()<<"端口数量一致,跳过更新";
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


void MainWindow::downloadFile()
{
    QByteArray fileContent;
    QString defaultFileName;

    // 获取当前选中的文件名
    QModelIndexList selectedIndexes = ui->fileListView->selectionModel()->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        QModelIndex index = selectedIndexes.first();
        QString selectedText = index.data(Qt::DisplayRole).toString();
        defaultFileName = selectedText.contains(":") ? selectedText.split(":").first().trimmed() : "";
    }

    for (int i = bufferList.size() - 1; i > 0; --i) {
        QString currentLine = bufferList.at(i);
        qDebug() << "currentLine " << currentLine << endl;
        if (currentLine == "__download__file__start__") {
            qDebug() << "发现start标记在 " << i << endl;
            for (int j = i; j < bufferList.size(); ++j) {
                currentLine = bufferList.at(j);
                // 如果当前行包含文件内容开始和结束的标记，则提取文件内容
                if (currentLine.contains("__download__line_start__") && currentLine.contains("__download__line_end__")) {
                    // 从当前行中提取文件内容并添加到fileContent
                    int startPos = currentLine.indexOf("__download__line_start__") + QString("__download__line_start__").length();
                    int endPos = currentLine.indexOf("__download__line_end__");
                    QByteArray lineContent = QByteArray::fromHex(currentLine.mid(startPos, endPos - startPos).toLatin1());
                    fileContent.append(lineContent);
                }
            }
            // 打开文件保存对话框
            QString fileName = QFileDialog::getSaveFileName(this, tr("保存文件"), defaultFileName);
            if (!fileName.isEmpty()) {
                QFile file(fileName);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(fileContent);
                    file.close();
                    qDebug() << "文件保存成功：" << fileName;
                } else {
                    qDebug() << "无法打开文件进行写入：" << fileName;
                }
            } else {
                qDebug() << "未选择文件保存路径";
            }
            return;
        }
    }
}




void MainWindow::listFileAction( )
{

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
            break;//这里需要跳出,不然会把历史记录全列出来了
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
           listFileAction();
       }

        if (completeData == "__download__file__end__" ) {


           downloadFile();
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


}
void  MainWindow::refreshViewGSV() {

}
void MainWindow::serialPortRecvData(QString cmd) {


    //qDebug()<<"SerialPort RecvData : "<<cmd;
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

}

void MainWindow::on_showLocate_stateChanged(int arg1)
{

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
    // QString filePath = QFileDialog::getSaveFileName(nullptr, "保存文件", "", "日志 (*.txt)");
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
    sendCommand(cmd);

    // 添加新项
//    QStringList newList;
//    newList << "Item 1" << "Item 2" << "Item 3";
//    fileListViewModel->setStringList(newList);




}




void MainWindow::on_baudRateComboBox_currentIndexChanged(const QString &arg1)
{
      MainWindow::on_openSerialPortButton_clicked();
}

void MainWindow::on_serialPortComboBox_currentIndexChanged(const QString &arg1)
{
    MainWindow::on_openSerialPortButton_clicked();
}
