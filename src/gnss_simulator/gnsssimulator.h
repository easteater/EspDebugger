#ifndef GNSSSIMULATOR_H
#define GNSSSIMULATOR_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <functional>

class GnssSimulator : public QObject
{
    Q_OBJECT

public:
    GnssSimulator();
    ~GnssSimulator();

    void setCallbackFunction(const std::function<void(const QString&)> &callbackFunction);
    void start();
    void stop();

private slots:
    void onTimerTimeout();

private:
    QStringList gnssList;
    QTimer *timer;
    std::function<void(const QString&)> callbackFunction;
};

#endif // GNSSSIMULATOR_H
