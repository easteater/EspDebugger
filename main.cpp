/***
  Partial code implementation and some translations from Chinese to English are referenced from GPT-3.5. Special thanks to GPT for its assistance.
  部分代码及中译英部 参考于GPT3.5 特此感谢GPT的帮助

*/
#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    //Define a translator, implement multi-language i18n. 定义翻译器,实现多国语言
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "untitled_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    //Declaration and display of the main window. 定义并显示主窗体
    MainWindow w;
    w.show();
    return a.exec();
}
