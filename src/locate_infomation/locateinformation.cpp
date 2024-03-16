/***
 *
 * */

#include "locateinformation.h"
#include "ui_locateinformation.h"


LocateInformation::LocateInformation(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocateInformation)
{
    ui->setupUi(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    // Open the debugger by enabling debugging. After launching, open the browser and enter 127.0.0.1:9901 to directly access the debugger. If you encounter any issues with content not displaying, open this option to identify the problem. Then, proceed to modify the corresponding HTML to resolve the issue.
    // 打开调试,启动后浏览器输入127.0.0.1:9901 就可以直接使用调试器了,如果你遇到不展示内容的话,就把这个打开,看下是什么问题, 然后去修改对应的html就可以了
    webView->setUrl(QUrl("qrc:/resource/locate.html"));
    // Set the size of the web view to match the window size, disable scrollbars, disable the right-click menu, and eliminate any gaps. 设置Web视图的大小与窗口一样大 禁用滚动条  禁用右键菜单 消除间隙
    webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    webView->settings()->setAttribute(QWebEngineSettings::ShowScrollBars, false);
    webView->setContextMenuPolicy(Qt::NoContextMenu);
    webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(webView);
    setLayout(layout);

}

LocateInformation::~LocateInformation()
{
    delete ui;
}
