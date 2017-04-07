#include "filetools.h"
#include <QtDebug>
#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include <QDir>

FileTools::FileTools(QObject *parent) : QObject(parent)
{

}

void FileTools::saveWaveToFile(qint32 *wvData, int len, int mode)
{
    this->recWaveData  = wvData;
    this->buf_size = len;
    this->mode_fpga = mode;

    QString filepath = getFilePath();       //得到文件保存的路径
    qDebug()<<filepath;

    QFile file;
    bool flag;

    //保存文本文件
    file.setFileName(filepath + ".txt");
    flag = file.open(QIODevice::WriteOnly | QIODevice::Text);
    if(flag){
        qDebug()<<"file open success! —— "<<filepath;
        QTextStream out(&file);
        short a;
        for(int i=0;i<buf_size;i++){
            a = (qint16)recWaveData[i];
            out << a <<",";
        }
        file.close();
    }
    else
        qDebug()<<"file open failed!";


    //保存二进制文件（小端）
    file.setFileName(filepath + ".DAT");
    flag = file.open(QIODevice::WriteOnly);
    if(flag){
        qDebug()<<"file open success! —— "<<filepath;
        QDataStream out(&file);
        out.setByteOrder(QDataStream::LittleEndian);
        short a;
        for(int i=0;i<buf_size;i++){
            out << (quint32)(i+1);
            out << (quint32)0xFF;
            a = (qint16)recWaveData[i];
            out << a;
        }
        file.close();
    }
    else
        qDebug()<<"file open failed!";

    saveCfgFile(filepath);      //生成对应的配置文件
}

//实现优先在SD卡建立波形存储文件夹
//如果SD卡未插入，则在内存当前文件夹下建立波形文件夹
//返回波形文件夹路径
QString FileTools::getFilePath()
{
    QString filename;
    filename = QString("%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss"));

    switch (mode_fpga) {
    case 0:     //地电波
        filename.prepend("TEV_");
        break;
    case 1:     //AA超声
        filename.prepend("AAUltrasonic_");
        break;
    case 2:     //

        break;
    default:
        break;
    }

    //文件夹操作
    QDir dir;
    if(dir.exists("/mmc/sdcard/WaveForm/")){
        return QString("/mmc/sdcard/WaveForm/" + filename);
    }
    else if(dir.mkdir("/mmc/sdcard/WaveForm/") ){
        return QString("/mmc/sdcard/WaveForm/" + filename);
    }
    else if(dir.exists("./WaveForm/") ){
        qDebug()<< "SD card failed!";
        return QString("./WaveForm/" + filename);
    }
    else if(dir.mkdir("./WaveForm/")){
        qDebug()<< "SD card failed!";
        return QString("./WaveForm/" + filename);
    }
    else{
        return QString();       //全失败不太可能
    }
}

void FileTools::saveCfgFile(QString str)
{
    QFile file;
    file.setFileName(str + ".CFG");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "wave1,,1999" << "\n";
    out << "1,1A,0D" << "\n";
    out << "1,UA,,,V,1.000000,0.000000,0,-32768,32767,1.000000,1.000000,S" << "\n";
    out << "50.000000" << "\n";
    out << "1" << "\n";
    out << "3200," << buf_size << "\n";     //点数可变
    out << QDateTime::currentDateTime().toString() << "\n";
    out << QDateTime::currentDateTime().toString() << "\n";
    out << "BINARY" << "\n";

    file.close();
}
