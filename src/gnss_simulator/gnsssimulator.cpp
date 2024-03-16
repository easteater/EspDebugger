#include "gnsssimulator.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QResource>
#include <QBuffer>

GnssSimulator::GnssSimulator()
    : timer(nullptr)
{
}

GnssSimulator::~GnssSimulator()
{
    if (timer) {
        timer->stop();
        delete timer;
    }
}

void GnssSimulator::setCallbackFunction(const std::function<void(const QString&)> &callbackFunction)
{
    this->callbackFunction = callbackFunction;
}

void GnssSimulator::start()
{
    if (!callbackFunction) {
        qDebug() << "回调函数未设置，请使用 setCallbackFunction 设置回调函数。";
        return;
    }



    // 创建定时器，每秒执行10次
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GnssSimulator::onTimerTimeout);
    timer->start(100);
}
void GnssSimulator::stop() {
    timer->stop();
}

void GnssSimulator::onTimerTimeout()
{
     if (gnssList.isEmpty()) {
        // 使用 QFile 打开资源中的gnss.txt文件
        QFile file(":/resource/gnss.txt");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "无法打开gnss.txt文件";
            return;
        }

        // 从文件读取内容并保存到列表中
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            gnssList.append(line);
        }

        // 如果gnssList为空，则无法执行后续操作
        if (gnssList.isEmpty()) {
            qDebug() << "gnss.txt文件内容为空";
            return;
        }


        qDebug() << "gnss.txt文件 已读取";
        file.close();
     }

    // 从gnssList中弹出一行数据并执行回调函数
    QString data = gnssList.takeFirst();
    callbackFunction(data + "\r\n");
   // qDebug() << "模拟器回调数据:"<<data;
}
