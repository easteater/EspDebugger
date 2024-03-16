
#include "signaltonoiseratio.h"
#include "ui_signaltonoiseratio.h"
#include <QtWebEngineWidgets>
#include <QWebEngineProfile>



SignaltoNoiseRatio::SignaltoNoiseRatio(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignaltoNoiseRatio)
{
    ui->setupUi(this);
    qDebug()<<"SignaltoNoiseRatio 已加载";
    QVBoxLayout* layout = new QVBoxLayout(this);
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "9901");
    webView->setUrl(QUrl("qrc:/resource/snr.html"));
    webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    webView->settings()->setAttribute(QWebEngineSettings::ShowScrollBars, false);
    webView->setContextMenuPolicy(Qt::NoContextMenu);
    webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(webView);
    setLayout(layout);



}

void SignaltoNoiseRatio::frefshChartView() {

    satelliteSignalList.clear();
    // Define an array of colors. 定义颜色数组
    QColor colors[] = {Qt::red, Qt::blue, Qt::green, Qt::yellow};
    QRandomGenerator randomGenerator;

    // Randomly generate 10 signals.
    for (int i = 0; i < 10; i++) {
        SatelliteSignal signal;
        signal.satelliteType = (i % 3 == 0) ? "GPS" : ((i % 3 == 1) ? "QZS" : "BD");
        signal.id = QString::number(randomGenerator.bounded(1, 101));  // Randomly generate a number between 1 and 100 as the ID. 随机生成1-100之间的数作为id
        signal.value = randomGenerator.bounded(101);  // Randomly generate a number between 0 and 100 as the value. 随机生成0-100之间的数作为value
        signal.color = colors[randomGenerator.bounded(4)];  // Randomly color

        satelliteSignalList.append(signal);
    }




}

SignaltoNoiseRatio::~SignaltoNoiseRatio()
{
    delete ui;
}

void SignaltoNoiseRatio::on_pushButton_clicked()
{
    satelliteSignalList.clear();
    QColor colors[] = {Qt::red, Qt::blue, Qt::green, Qt::yellow};
    QRandomGenerator randomGenerator;
    for (int i = 0; i < 10; i++) {
        SatelliteSignal signal;
        signal.satelliteType = (i % 3 == 0) ? "GPS" : ((i % 3 == 1) ? "QZS" : "BD");
        signal.id = QString::number(randomGenerator.bounded(1, 101));  // 随机生成1-100之间的数作为id
        signal.value = randomGenerator.bounded(101);  // 随机生成0-100之间的数作为value
        signal.color = colors[randomGenerator.bounded(4)];  // 随机选择一个颜色
        satelliteSignalList.append(signal);
    }


}

