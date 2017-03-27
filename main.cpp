#include <QSplashScreen>
#include <QDesktopWidget>
#include <QFontDatabase>
#include <QApplication>
#include <QTextCodec>
#include <QThread>
#include <QTranslator>
#include "Gui/mainwindow.h"
#include "IO/SqlCfg/sqlcfg.h"

void print(void);
void print_centor(void);

int main(int argc, char *argv[])
{
    int fontid;

    /* System print */
    print_centor();

    /* Sqlite3 init */
    sqlite3_init();

    QApplication a(argc, argv);


//定义字体
#ifdef ARM
    fontid = QFontDatabase::addApplicationFont("/usr/local/QtEmbedded-5.6.0/lib/fonts/wenquanyi_10pt.bdf");
//    fontid = QFontDatabase::addApplicationFont("/home/zdit/prog/work/swbox/resource/wenquanyi.ttf");

#else
    fontid = QFontDatabase::addApplicationFont("/usr/local/Qt-5.6.0/lib/fonts/wenquanyi_10pt.bdf");
#endif
    QString myfont = QFontDatabase::applicationFontFamilies(fontid).at(0);
    QFont font(myfont,10);
    QApplication::setFont(font);

    qDebug()<<"fontid = "<<fontid;
    qDebug()<<QFontDatabase::applicationFontFamilies(fontid);

    //语言切换模块

    qDebug("language = %s", sqlcfg->get_para()->language == EN ? "EN" : "CN");

    if(sqlcfg->get_para()->language == LANGUAGE::EN){
        QTranslator *translator = new QTranslator(qApp);
        bool flag = translator->load(QString("/root/trans/en.qm"));
        qDebug()<<"load language mode..."<<flag;
        qApp->installTranslator(translator);
    }


    MainWindow w;
    w.show();

    return a.exec();
}

void print_centor(void)
{
#ifdef ARM
    const static char *str = "ARM";
#else
    const static char *str = "PC";
#endif
    printf("\n=================================================\n");
    printf("\t\tQT PROJ : swbox\n");
    printf("\t     QT version : %d.%d.%d\n", QT_VERSION >> 16,
           0xf & (QT_VERSION >> 8), 0xf & QT_VERSION);
    printf("\t QT GUI version : Developping...\n");
    printf("\t    QT platform : %s\n", str);
    printf("      QT GUI Copyrights : * * *\n");
    printf("\t  QT GUI Author : LPC\n");
    printf("\tQT GUI Compiled : %s | %s\n", __TIME__, __DATE__);
    printf("=================================================\n\n");
    printf("  QT GUI for swbox starting ...\n\n\n");
}
