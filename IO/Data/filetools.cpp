#include "filetools.h"
#include <QtDebug>
#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include "IO/SqlCfg/sqlcfg.h"


//还没有开启冲突控制
FileTools::FileTools(VectorList data, MODE mode)
{
    _data = data;
    _mode = mode;
}

FileTools::~FileTools()
{
//    qDebug()<<"new thread save completed! mode="<<_mode;
}

void FileTools::run()
{
    filepath = getFilePath();       //得到文件保存的路径
    qDebug()<<filepath;

    saveDataFile();     //保存数据文件
    saveCfgFile();      //生成对应的配置文件

    if(_mode == AA_Ultrasonic){
        saveWavFile();      //生成声音文件
        //这里插入mp3转换代码
        wavToMp3();
    }

    //空间管理
    spaceControl("/root/WaveForm/");
    spaceControl("/mmc/sdcard/WaveForm/");

    system ("sync");

}



void FileTools::saveDataFile()
{
    QFile file;
    bool flag;

    //    //保存文本文件
    //    file.setFileName(filepath + ".txt");
    //    flag = file.open(QIODevice::WriteOnly | QIODevice::Text);
    //    if(flag){
    //        QTextStream out(&file);
    //        short a;
    //        for(int i=0;i<_data.length();i++){
    //            a = (qint16)_data[i];
    //            out << a <<",";
    //        }
    //        qDebug()<<file.fileName()<<"    "<<_data.length()<<" points";
    //        file.close();
    //    }
    //    else
    //        qDebug()<<"file open failed!";


    //保存二进制文件（小端）
    file.setFileName(filepath + ".DAT");
    flag = file.open(QIODevice::WriteOnly);
    if(flag){
        QDataStream out(&file);
        out.setByteOrder(QDataStream::LittleEndian);
        short a;
        if(_mode == TEV_Double){
            for(int i=0;i<_data.length()/2;i++){
                out << (quint32)(i+1);
                out << (quint32)0xFF;
                a = (qint16)_data[i];
                out << a;
                a = (qint16)_data[i + _data.length()/2];
                out << a;
            }
        }
        else {
            for(int i=0;i<_data.length();i++){
                out << (quint32)(i+1);
                out << (quint32)0xFF;
                a = (qint16)_data[i];
                out << a;
            }
        }


        if(file.copy("/mmc/sdcard/WaveForm/" + filename + ".DAT")){
//            qDebug()<<"copy " + filename + ".DAT to SDCard succeed!";
        }
        else{
            qDebug()<<"copy " + filename + ".DAT to SDCard failed!";
        }

        file.close();
    }
    else
        qDebug()<<"file open failed!";



}

//实现优先在SD卡建立波形存储文件夹
//如果SD卡未插入，则在内存当前文件夹下建立波形文件夹
//返回波形文件夹路径
QString FileTools::getFilePath()
{
//    QString filename;
    filename = QString("%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz"));

    switch (_mode) {
    case TEV1:     //TEV1
        filename.prepend("TEV1_");
        break;
    case TEV2:     //TEV2
        filename.prepend("TEV2_");
        break;
    case TEV_Double:     //TEV_Double
        filename.prepend("TEVDouble_");
        break;
    case AA_Ultrasonic:     //AA超声
        filename.prepend("AAUltrasonic_");
        break;
    case RFCT:
        filename.prepend("RFCT_");
        break;
    case RFCT_CONTINUOUS:
        filename.prepend("RFCT_CONTINUOUS_");
    default:
        break;
    }

    //文件夹操作
    QDir dir;
    //创建文件夹
    if(!dir.exists("/mmc/sdcard/WaveForm/")){
        dir.mkdir("/mmc/sdcard/WaveForm/");
    }


    //    if(dir.exists("/mmc/sdcard/WaveForm/")){
    //        return QString("/mmc/sdcard/WaveForm/" + filename);
    //    }
    //    else if(dir.mkdir("/mmc/sdcard/WaveForm/") ){
    //        return QString("/mmc/sdcard/WaveForm/" + filename);
    //    }
    //    else
    if(dir.exists("/root/WaveForm/") ){
        //        qDebug()<< "SD card failed!";
        return QString("/root/WaveForm/" + filename);
    }
    else if(dir.mkdir("/root/WaveForm/")){
        //        qDebug()<< "SD card failed!";
        return QString("/root/WaveForm/" + filename);
    }
    else{
        return QString();       //全失败不太可能
    }
}

void FileTools::saveCfgFile()
{
    QFile file;
    file.setFileName(filepath + ".CFG");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "wave1,,1999" << "\n";
    if(_mode == TEV_Double){
        out << "2,2A,0D" << "\n";
        out << "1,UA,,,V,1.000000,0.000000,0,-32768,32767,1.000000,1.000000,S" << "\n";
        out << "2,UB,,,V,1.000000,0.000000,0,-32768,32767,1.000000,1.000000,S" << "\n";
        out << sqlcfg->get_para()->freq_val << ".000000" << "\n";
        out << "1" << "\n";
        out << "3200," << _data.length()/2 << "\n";     //点数可变
    }
    else{
        out << "1,1A,0D" << "\n";
        out << "1,UA,,,V,1.000000,0.000000,0,-32768,32767,1.000000,1.000000,S" << "\n";
        out << sqlcfg->get_para()->freq_val << ".000000" << "\n";
        out << "1" << "\n";
        out << "3200," << _data.length() << "\n";     //点数可变
    }


    out << QDateTime::currentDateTime().toString() << "\n";
    out << QDateTime::currentDateTime().toString() << "\n";
    out << "BINARY" << "\n";

    if(file.copy("/mmc/sdcard/WaveForm/" + filename + ".CFG")){
//        qDebug()<<"copy " + filename + ".CFG to SDCard succeed!";
    }
    else{
        qDebug()<<"copy " + filename + ".CFG to SDCard failed!";
    }

    file.close();
}

//保存wav文件
void FileTools::saveWavFile()
{
	int i, size;
	
    wfh1=(WaveFormat*)malloc(sizeof(WaveFormat));
	if (wfh1 == NULL) {
		return;
	}
    //    memset(wfh1,0,sizeof(wfh1));

    size = _data.length() * 2;
    //    char *Data1 = (char*)malloc(size);
    //    for(int i=0;i<size;i++){
    //        Data1[i] = (char)_data.at(i);
    //    }



    strcpy(wfh1->RIFF,"RIFF");
    wfh1->Filesize = size + sizeof(WaveFormat);
    strcpy(wfh1->WavFlags,"WAVE");
    strcpy(wfh1->FMTFlags,"fmt ");
    wfh1->ByteFilter = 16;
    wfh1->Format = 1;
    wfh1->Channel = 1;
    wfh1->SampleFreq = 320000;
    wfh1->PCMBitSize = 16;
    wfh1->ByteFreq   = wfh1->PCMBitSize * wfh1->Channel * wfh1->SampleFreq / 8;     //640000
    wfh1->BlockAlign = wfh1->PCMBitSize * wfh1->Channel / 8;
    strcpy(wfh1->fact,"data");
    wfh1->Datasize = size;


    QFile file("out.wav");
    //    file.setFileName(filepath + ".wav");
    file.setFileName("/tmp/out.wav");

    if (!file.open(QIODevice::WriteOnly))
        return;

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);

    out.writeRawData(wfh1->RIFF,4);
    out << (qint32)wfh1->Filesize;
    out.writeRawData(wfh1->WavFlags,4);
    out.writeRawData(wfh1->FMTFlags,4);
    out << wfh1->ByteFilter;
    out << wfh1->Format;
    out << wfh1->Channel;
    out << (qint32)wfh1->SampleFreq;
    out << (qint32)wfh1->ByteFreq;
    out << wfh1->BlockAlign;
    out << wfh1->PCMBitSize;
    out.writeRawData(wfh1->fact,4);
    out << (qint32)wfh1->Datasize;

    qDebug()<<"wfh1->Datasize :"<< wfh1->Datasize;

    //    char c;
    //    int t;
    for(i=0;i<_data.length();i++){
        out << qint16(_data.at(i) /** 32*/ << 5);
        //        t = _data.at(i)>>2;
        //        c = char(t);
        //        out.writeRawData(&c,1);
    }
    qDebug()<<"i="<<i;

    file.close();

	free (wfh1);
}

void FileTools::wavToMp3()
{
    QString program = "/root/lame";
    QStringList arguments1,arguments2;
    arguments1 << "/tmp/out.wav" << filepath+".mp3";
    arguments2 << "/tmp/out.wav" << "/mmc/sdcard/WaveForm/"+ filename +".mp3";

    QProcess *myProcess1 = new QProcess;
    myProcess1->start(program, arguments1);
    QObject::connect(myProcess1,SIGNAL(finished(int)),myProcess1,SLOT(deleteLater()));

    QProcess *myProcess2 = new QProcess;
    myProcess2->start(program, arguments2);
    qDebug()<<"mp3 file saved!";
    QObject::connect(myProcess2,SIGNAL(finished(int)),myProcess2,SLOT(deleteLater()));
}

void FileTools::spaceControl(QString str)
{
    QDir dir(str);
    QStringList list = dir.entryList(QDir::Files,QDir::Time);
    if(list.length()>sqlcfg->get_para()->max_rec_num){
        for(int i=0;i<12;i++){          //删除10个文件
            QString s = list.at(i);
            if(dir.remove(s)){
                qDebug()<<"remove file "<<str + s<<" succeed!";
            }
            else{
                qDebug()<<"remove file "<<str + s<<" failed!";
            }
        }

//		system ("sync");
    }
}



