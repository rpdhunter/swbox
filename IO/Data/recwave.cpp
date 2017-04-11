#include "recwave.h"
#include <QtDebug>
#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QThread>

RecWave::RecWave(G_PARA *gdata, MODE mode, QObject *parent) : QObject(parent)
{
    num =0;
    tdata = gdata;
    this->mode = mode;
    status = Free;

    timer = new QTimer;
    timer->setSingleShot(true);
    timer->setInterval(10000 );  //录5秒钟超声

    if(mode == TEV){
        this->groupNum = 0;
    }
    else if(mode == AA_Ultrasonic){
        this->groupNum = 0x200;
    }


    connect(timer,SIGNAL(timeout()),this,SLOT(AA_rec_end()));
//    connect(this,SIGNAL(waveData(VectorList,MODE)),this,SLOT(saveWaveToFile()));

}

//从GUI发起录波指令
void RecWave::recStart()
{
    if(mode == TEV){
        tdata->send_para.recstart.rval = 1;
    }
    else if(mode == AA_Ultrasonic){
        tdata->send_para.recstart.rval = 4;
        timer->start();
    }

    tdata->send_para.recstart.flag = true;

}

//上传数据的准备工作
void RecWave::startWork()
{
    _data.clear();
    time = QTime::currentTime();



    if(mode == TEV){
        tdata->send_para.recstart.rval = 0x10;     //数据上传开始
        tdata->send_para.recstart.flag = true;
        this->groupNum = 0;

        tdata->send_para.groupNum.rval = 0;
        tdata->send_para.groupNum.flag = true;
    }
    else if(mode == AA_Ultrasonic){
        num = 0;
        this->groupNum = 0x200;

        tdata->send_para.groupNum.rval = 0x200;
        tdata->send_para.groupNum.flag = true;
    }

}

//上传数据
void RecWave::work()
{
    if(mode == TEV){
        if(tdata->recv_para.groupNum == tdata->send_para.groupNum.rval ){      //收发相匹配，拷贝数据
            for(int i=0;i<256;i++){
                _data.append((qint32)tdata->recv_para.recData [ i + 1 ] - 0x8000);
                //                qDebug("%08X",tdata->recv_para.recData [ i ]);
            }

            printf("receive recWaveData! send_groupNum=%d\n",tdata->send_para.groupNum.rval);

            this->groupNum ++;
            tdata->send_para.groupNum.rval = groupNum;
            tdata->send_para.groupNum.flag = true;
        }
        else{                                                               //不匹配，再发一次
            printf("receive recWaveData failed! send groupNum=%d\n",tdata->send_para.groupNum.rval);
            printf("recv groupNum=%d\n",tdata->recv_para.groupNum);
            tdata->send_para.groupNum.flag = true;
        }

        if(tdata->send_para.groupNum.rval == GROUP_NUM_MAX){        //接收组装数据完毕
            tdata->send_para.recstart.rval=0;
            tdata->send_para.recstart.flag = true;

            tdata->send_para.groupNum.flag = false;

            qDebug()<<QString("rec wave cost time: %1 ms").arg( - QTime::currentTime().msecsTo(time));
            qDebug()<<"receive recWaveData complete!";
            // 录波完成，发送数据，通知GUI和文件保存
            emit waveData(_data,mode);
            status = Free;
        }
    }
    else if(mode == AA_Ultrasonic){

        if((tdata->recv_para.groupNum + 0x200) == tdata->send_para.groupNum.rval ){      //收发相匹配，拷贝数据
            for(int i=0;i<256;i++){
//                _data.append((qint32)tdata->recv_para.recData [ i + 1 ] - 0x8000);
//                _data.append((qint32)tdata->recv_para.recData [ i + 1 ]);
//                _data.append(0);
                num++;

                //                qDebug("%08X",tdata->recv_para.recData [ i ]);
            }


//            printf("receive recWaveData! send_groupNum=%d\n",tdata->send_para.groupNum.rval);

            ++groupNum;

            tdata->send_para.groupNum.rval = groupNum;
            tdata->send_para.groupNum.flag = true;
        }
        else{                                                               //不匹配，再发一次
            printf("receive recWaveData failed! send groupNum=%d\n",tdata->send_para.groupNum.rval);
            printf("recv groupNum=%d\n",tdata->recv_para.groupNum);
            tdata->send_para.groupNum.flag = true;
        }

        if(tdata->send_para.groupNum.rval == 0x200 + GROUP_NUM_MAX){

            this->groupNum = 0x200;

            tdata->send_para.groupNum.flag = true;
            tdata->send_para.groupNum.rval = groupNum;

        }
    }

}

void RecWave::AA_rec_end()
{
    tdata->send_para.groupNum.rval = 0 ;
    tdata->send_para.groupNum.flag = true;
    tdata->send_para.recstart.rval = 0;
    tdata->send_para.recstart.flag = true;

    qDebug()<<"                                  receive recAAData complete!";
    //录波完成，发送数据，通知GUI和文件保存
    emit waveData(_data,mode);
    status = Free;
}

void RecWave::saveWaveToFile()
{
    QString filepath = getFilePath();       //得到文件保存的路径
    qDebug()<<filepath;

    QFile file;
    bool flag;

    //保存文本文件
    file.setFileName(filepath + ".txt");
    flag = file.open(QIODevice::WriteOnly | QIODevice::Text);
    if(flag){      
        QTextStream out(&file);
        short a;
        for(int i=0;i<_data.length();i++){
            a = (qint16)_data[i];
            out << a <<",";
        }
//        qDebug()<<"file save success! —— "<<file.fileName()<<"    "<<_data.length()<<"points";
        qDebug()<<"file save success! —— "<<file.fileName()<<"    "<<num<<"points";
        file.close();
    }
    else
        qDebug()<<"file open failed!";


    //保存二进制文件（小端）
    file.setFileName(filepath + ".DAT");
    flag = file.open(QIODevice::WriteOnly);
    if(flag){
        QDataStream out(&file);
        out.setByteOrder(QDataStream::LittleEndian);
        short a;
        for(int i=0;i<_data.length();i++){
            out << (quint32)(i+1);
            out << (quint32)0xFF;
            a = (qint16)_data[i];
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
QString RecWave::getFilePath()
{
    QString filename;
    filename = QString("%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz"));

    switch (mode) {
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

void RecWave::saveCfgFile(QString str)
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
    out << "3200," << _data.length() << "\n";     //点数可变
    out << QDateTime::currentDateTime().toString() << "\n";
    out << QDateTime::currentDateTime().toString() << "\n";
    out << "BINARY" << "\n";

    file.close();
}



