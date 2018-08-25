#include <QApplication>
#include <QFontDatabase>
#include <QTranslator>
#include <QSplashScreen>
#include <QQmlContext>
#include <QFile>
#include <QDir>

#include "Gui/mainwindow.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "Gui/Qml/quickview.h"
#include "IO/Other/cpu.h"

#include <QSqlDatabase>

void print_centor();

int main(int argc, char *argv[])
{
    set_process_cpu(0,CPU_0);

    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    QApplication a(argc, argv);
//    QCoreApplication::setOrganizationName("ZDIT");
//    QCoreApplication::setApplicationName("swbox");

#ifdef OHV
    QPixmap pixmap(":/widgetphoto/bk/ohv.png");
#elif AMG
    QPixmap pixmap(":/widgetphoto/bk/amg.png");
#elif ZDIT
    QPixmap pixmap(":/widgetphoto/bk/zdit.png");
#elif XDP_II
    QPixmap pixmap(":/widgetphoto/bk/xdp2.png");
#else
    QPixmap pixmap(":/widgetphoto/bk/powersystem.png");
#endif

    QFile file("/root/OEM/logo.png");
    if(file.exists()){
        pixmap.load("/root/OEM/logo.png");
    }

    pixmap.scaled(480,272);
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->show();

    print_centor();
    sqlcfg = new SqlCfg();

    splash->showMessage(QObject::tr("正在载入语言模块..."),Qt::AlignBottom|Qt::AlignLeft);
    qDebug("language = %s", sqlcfg->get_para()->language == EN ? "EN" : "CN");
    if(sqlcfg->get_para()->language == LANGUAGE::EN){
        QTranslator *translator = new QTranslator(qApp);
        bool flag = translator->load(QString("/root/trans/en.qm"));
        qDebug()<<"load language mode..."<<flag;
        qApp->installTranslator(translator);
    }

    splash->showMessage(QObject::tr("正在初始化字体"),Qt::AlignBottom|Qt::AlignLeft);
    int fontid = QFontDatabase::addApplicationFont("/usr/local/QtEmbedded-5.9.1/lib/fonts/wenquanyi_10pt.bdf");
    QString myfont = QFontDatabase::applicationFontFamilies(fontid).at(0);
    QFont font(myfont,10);
    QApplication::setFont(font);

    splash->showMessage(QObject::tr("正在初始化主窗体..."),Qt::AlignBottom|Qt::AlignLeft);
    MainWindow w(splash);
    w.show();

    splash->showMessage(QObject::tr("正在初始化QML..."),Qt::AlignBottom|Qt::AlignLeft);
    QuickView view;
    view.rootContext()->setContextProperty("dt",&view);
    view.setSource(QString("qrc:/Input.qml"));
    view.hide();
    QObject::connect(&w,SIGNAL(show_input(QString,QString)), &view, SLOT(show_input(QString,QString)) );
    QObject::connect(&w,SIGNAL(send_input_key(quint8)), &view, SLOT(trans_input_key(quint8)) );
    QObject::connect(&view,SIGNAL(input_str(QString)), &w, SIGNAL(input_str(QString)) );

    splash->finish(&w);
    delete splash;

    return a.exec();
}

void print_centor()
{
    const static char *str = "ARM";
    printf("\n=================================================\n");
    printf("\t\tQT PROJ : swbox\n");
    printf("\t     QT version : %d.%d.%d\n", QT_VERSION >> 16,
           0xf & (QT_VERSION >> 8), 0xf & QT_VERSION);
    printf("\t QT GUI version : %d.%d\n",VERSION_MAJOR,VERSION_MINOR);
    printf("\t    QT platform : %s\n", str);
    printf("      QT GUI Copyrights : zdit\n");
    printf("\t  QT GUI Author : RPD\n");
    printf("\tQT GUI Compiled : %s | %s\n", __TIME__, __DATE__);
    printf("=================================================\n\n");
    printf("  QT GUI for swbox starting ...\n\n\n");
}
