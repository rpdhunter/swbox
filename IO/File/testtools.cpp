#include "testtools.h"
#include <QFile>
#include <QTextStream>
#include "Gui/Common/common.h"
#include <QDateTime>
#include <QtDebug>

TestTools::TestTools(QObject *parent) : QObject(parent)
{

}

void TestTools::save_test_file(QVector<int> list, QString path)
{
    if(list.isEmpty()){
        return;
    }

    Common::mk_dir(DIR_TEST);

    QFile file;
    file.setFileName(path + "/test_" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz") + ".txt" );
    bool flag = file.open(QIODevice::WriteOnly);

    if(flag){
        QTextStream out(&file);

        foreach (int d, list) {
            out << d << "\n";
        }

//        out << "\n";


        qDebug()<<"save test file :" << file.fileName();

        file.close();
    }
    else{
        qDebug()<<"log file saved failed! ";
    }
}

void TestTools::save_test_file(QVector<QPoint> list, QString path)
{
    if(list.isEmpty()){
        return;
    }

    Common::mk_dir(DIR_TEST);

    QFile file;
    file.setFileName(path + "/test_" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz") + ".txt" );
    bool flag = file.open(QIODevice::WriteOnly);

    if(flag){
        QTextStream out(&file);

        foreach (QPoint d, list) {
            out << d.x()%360 << "\t" << d.y() << "\t" << 1 <<"\n";
        }

//        out << "\n";


        qDebug()<<"save test file :" << file.fileName();

        file.close();
    }
    else{
        qDebug()<<"log file saved failed! ";
    }
}
