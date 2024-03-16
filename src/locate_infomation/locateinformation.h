#ifndef LOCATEINFORMATION_H
#define LOCATEINFORMATION_H

#include <QDialog>
#include <QtWebEngineWidgets>
#include <QWebEngineProfile>

namespace Ui {
class LocateInformation;
}

class LocateInformation : public QDialog
{
    Q_OBJECT

public:
    explicit LocateInformation(QWidget *parent = nullptr);
    ~LocateInformation();
     QWebEngineView *webView= new  QWebEngineView();

private:
    Ui::LocateInformation *ui;

    QLayout *layout;
    QTimer *frefshChartViewTimer;
};

#endif // LOCATEINFORMATION_H
