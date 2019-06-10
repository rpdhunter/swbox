#include "spacecontrol.h"
#include <QDir>
//#include <QFileInfoList>
#include <QtDebug>
#include <QProcess>
#include "Gui/Common/common.h"

SpaceControl::SpaceControl(QObject *parent) : QObject(parent)
{

}

void SpaceControl::file_bySize(QString path, quint64 size)
{
//    qDebug()<<"dirFileSize:"<<dirFileSize(path) << "\ttar :"<< size;
    while(dirFileSize(path) > size){
        removeOldFile_byNum(path, 1);
//        qDebug()<<"dirFileSize:"<<dirFileSize(path) << "\ttar :"<< size;
    }
}

void SpaceControl::dir_byNum(QString path, int num)
{
    QDir dir(path);
    QFileInfoList list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);


    while(list.count() > num){
        Common::del_dir(list.last().absoluteFilePath() );
        qDebug()<<"del_dir:"<<list.last().absoluteFilePath();
        list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Time);

    }

//    foreach (QFileInfo info, list) {
//        qDebug()<<info.absoluteFilePath();
//    }

}

void SpaceControl::removeOldFile_byNum(QString path, int num)
{
    QDir dir(path);
//    QStringList filters;
//    filters << "*.DAT" << "*.dat";
    QStringList list = dir.entryList(QDir::Files,QDir::Time);

//    qDebug()<<"max_rec_num:"<<sqlcfg->get_para()->max_rec_num << "\tcurrent"<<list.length();

    //按文件数目删除
    num = num>list.count() ? list.count() : num;
    for(int i=0;i<num;i++){          //删除12个文件
        QString s = list.at(list.length() - i - 1);
        dir.remove(s);
        qDebug()<<"remove file:"<<s;
    }
}

/************************************************************
 * 清理根目录,规则如下
 * 1.根目录下只保留Datalog,RPRDLog,Asset,Waveform四个文件夹
 * 2.每个文件夹分别设定不同的限额,不超过限额则不动,超过限额则删除到满足限额为止
 * 3.Datalog,RPRDLog删除所有文件夹,Asset删除所有文件,Waveform删除除favorite的文件夹
 * **********************************************************/
void SpaceControl::removeOldFile_smart()
{
    QDir dir(DIR_DATA);
    QFileInfoList list = dir.entryInfoList();
    foreach (QFileInfo info, list) {
//        qDebug()<<info.absoluteDir() << '\t' << info.absoluteFilePath() <<;

    }
}

quint64 SpaceControl::dirFileSize(const QString &path)
{
    QDir dir(path);
    quint64 size = 0;
    //dir.entryInfoList(QDir::Files)返回文件信息
    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Files))
    {
        //计算文件大小
        size += fileInfo.size();
    }
    //dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot)返回所有子目录，并进行过滤
    foreach(QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        //若存在子目录，则递归调用dirFileSize()函数
        size += dirFileSize(path + QDir::separator() + subDir);
    }
    return size;
}

QString SpaceControl::size_to_string(double size)
{
    if(size < 1000){
        return QString("%1B").arg(QString::number(size,'f',1));
    }
    else if(size < 1e6){
        return QString("%1K").arg(QString::number(size/1.0e3,'f',1));
    }
    else if(size < 1e9){
        return QString("%1M").arg(QString::number(size/1.0e6,'f',1));
    }
    else{
        return QString("%1G").arg(QString::number(size/1.0e9,'f',1));
    }
}

//返回磁盘信息,使用了linux命令"df"
void SpaceControl::disk_info(qint64 &total, qint64 &used, qint64 &available, QString &persent)
{
    QProcess process;
    process.start("df /mmc/mmc2");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString str_output = output.simplified();
    QStringList list = str_output.split(' ');
//    total = list.at(8).toLong();
    used = list.at(9).toLong();
    available = list.at(10).toLong();

    total = used + available;
    persent = list.at(11);
//    qDebug()<<list;
}











