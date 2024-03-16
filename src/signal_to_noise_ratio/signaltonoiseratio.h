#ifndef SIGNALTONOISERATIO_H
#define SIGNALTONOISERATIO_H

#include <QDialog>
#include <QtWebView/QtWebView>
#include <QVBoxLayout>
#pragma comment(lib,"Qt5Widgets.lib")
#pragma comment(lib,"Qt5WebKitWidgets.lib")

#include <QtWebEngineWidgets>
#include <QWebEngineProfile>


struct SatelliteSignal {
    QString satelliteType;
    QString id;
    int value;
    QColor color;
};


namespace Ui {
class SignaltoNoiseRatio;
}

class SignaltoNoiseRatio : public QDialog
{
    Q_OBJECT

public:
    explicit SignaltoNoiseRatio(QWidget *parent = nullptr);
    ~SignaltoNoiseRatio();
    QVector<SatelliteSignal> satelliteSignalList;
    QWebEngineView *webView= new  QWebEngineView();

private slots:
    void frefshChartView();
    void on_pushButton_clicked();


private:
    Ui::SignaltoNoiseRatio *ui;
    QLayout *layout;
    QTimer *frefshChartViewTimer;
};

#endif // SIGNALTONOISERATIO_H
