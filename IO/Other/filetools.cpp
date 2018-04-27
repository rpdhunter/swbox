#include "filetools.h"
#include <QtDebug>
#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include "IO/SqlCfg/sqlcfg.h"


//还没有开启冲突控制
FileTools::FileTools(VectorList data, MODE mode, FileMode filemode)
{
    _data = data;
    _mode = mode;
    _fileMode = filemode;
    mode_envelope_modify();
}

FileTools::FileTools(QString str,FileTools::FileMode filemode)
{
    getReadFilePath(str);
    _fileMode = filemode;
    mode_envelope_modify();
}

FileTools::~FileTools()
{
    //    qDebug()<<"new thread save completed! mode="<<_mode;
}

void FileTools::run()
{
    if(_fileMode == Write){
        getWriteFilePath();          //得到文件保存的路径
        saveDataFile();         //保存数据文件
        saveCfgFile();          //生成对应的配置文件

        if(_mode == AA1 || _mode == AA2 || _mode == AE1 || _mode == AE2
                || _mode == AA1_ENVELOPE || _mode == AA2_ENVELOPE || _mode == AE1_ENVELOPE || _mode == AE2_ENVELOPE){
            saveWavFile();      //生成声音文件
            wavToMp3();         //mp3转换
//            wav_add_filter();       //TEST
        }

        //空间管理
        spaceControl(WAVE_DIR"/");              //内存空间管理
        spaceControl("/mmc/sdcard/WaveForm/");  //SD卡空间管理

        system ("sync");
    }
    else if(_fileMode == Read){
        QFile file;
        file.setFileName(filepath);

        if (!file.open(QIODevice::ReadOnly)){
            qDebug()<<"file open failed!";
            return;
        }

        QDataStream in(&file);
        in.setByteOrder(QDataStream::LittleEndian);

        quint32 t1,t2;
        qint16 v;
        _data.clear();

        while (!in.atEnd()) {
            in >> t1 >> t2 >> v;
            _data.append(v);
            if( _mode == Double_Channel){
                in >> v;
                _data.append(v);
            }
        }
        emit readFinished(_data, _mode);

        if(_mode == AA1 || _mode == AA2 || _mode == AE1 || _mode == AE2         //TEST
                || _mode == AA1_ENVELOPE || _mode == AA2_ENVELOPE || _mode == AE1_ENVELOPE || _mode == AE2_ENVELOPE){
            if(filepath.contains("_env")){
                saveWavFile();      //生成声音文件
                wavToMp3();         //mp3转换
            }

        }

    }


}


void FileTools::mode_envelope_modify()
{
    if(_mode == AA1 && sqlcfg->get_para()->aa1_sql.envelope == true){
        _mode = AA1_ENVELOPE;
    }
    else if(_mode == AA2 && sqlcfg->get_para()->aa2_sql.envelope == true){
        _mode = AA2_ENVELOPE;
    }
    else if(_mode == AE1 && sqlcfg->get_para()->ae1_sql.envelope == true){
        _mode = AE1_ENVELOPE;
    }
    else if(_mode == AE2 && sqlcfg->get_para()->ae2_sql.envelope == true){
        _mode = AE2_ENVELOPE;
    }
}

void FileTools::getReadFilePath(QString str)
{
    if(str.contains("TEV1_")){
        _mode = TEV1;
    }
    else if(str.contains("TEV2_")){
        _mode = TEV2;
    }
    else if(str.contains("HFCT1_")){
        _mode = HFCT1;
    }
    else if(str.contains("HFCT2_")){
        _mode = HFCT2;
    }
    else if(str.contains("UHF1_")){
        _mode = UHF1;
    }
    else if(str.contains("UHF2_")){
        _mode = UHF2;
    }
    else if(str.contains("AA1_")){
        _mode = AA1;
    }
    else if(str.contains("AA2_")){
        _mode = AA2;
    }
    else if(str.contains("AE1_")){
        _mode = AE1;
    }
    else if(str.contains("AE2_")){
        _mode = AE2;
    }
    else if(str.contains("Double_")){
        _mode = Double_Channel;
    }

    if(str.contains(QString("☆") )){
        filepath = QString(FAVORITE_DIR"/" + str.remove(QString("☆") ) + ".DAT");        //收藏夹
    }
    else{
        filepath = QString(WAVE_DIR"/"+str+".DAT");
    }
}

//实现优先在SD卡建立波形存储文件夹
//如果SD卡未插入，则在内存当前文件夹下建立波形文件夹
//返回波形文件夹路径
void FileTools::getWriteFilePath()
{
    filename = QString("%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz"));

    switch (_mode) {
    case TEV1:
        filename.prepend("TEV1_");
        break;
    case TEV2:
        filename.prepend("TEV2_");
        break;
    case Double_Channel:
        filename.prepend("Double_");
        break;
    case AA1:
        filename.prepend("AA1_");
        break;
    case AA2:
        filename.prepend("AA2_");
        break;
    case AE1:
        filename.prepend("AE1_");
        break;
    case AE2:
        filename.prepend("AE2_");
        break;
    case HFCT1:
        filename.prepend("HFCT1_");
        break;
    case HFCT2:
        filename.prepend("HFCT2_");
        break;
    case UHF1:
        filename.prepend("UHF1_");
        break;
    case UHF2:
        filename.prepend("UHF2_");
        break;
    case TEV1_CONTINUOUS:
        filename.prepend("TEV1_CONTINUOUS_");
        break;
    case TEV2_CONTINUOUS:
        filename.prepend("TEV2_CONTINUOUS_");
        break;
    case HFCT1_CONTINUOUS:
        filename.prepend("HFCT1_CONTINUOUS_");
        break;
    case HFCT2_CONTINUOUS:
        filename.prepend("HFCT2_CONTINUOUS_");
        break;
    case UHF1_CONTINUOUS:
        filename.prepend("UHF1_CONTINUOUS_");
        break;
    case UHF2_CONTINUOUS:
        filename.prepend("UHF2_CONTINUOUS_");
        break;
    case AA1_ENVELOPE:
        filename.prepend("AA1_ENVELOPE_");
        break;
    case AA2_ENVELOPE:
        filename.prepend("AA2_ENVELOPE_");
        break;
    case AE1_ENVELOPE:
        filename.prepend("AE1_ENVELOPE_");
        break;
    case AE2_ENVELOPE:
        filename.prepend("AE2_ENVELOPE_");
        break;
    default:
        break;
    }

    //文件夹操作
    QDir dir;
    //创建文件夹
    if(!dir.exists("/mmc/sdcard/WaveForm/")){
        dir.mkdir("/mmc/sdcard/WaveForm/");
    }

    if(!dir.exists("/mmc/sdcard/WaveForm/favorite")){
        dir.mkdir("/mmc/sdcard/WaveForm/favorite");
    }

    if(!dir.exists(USB_DIR"/") ){
        dir.mkdir(USB_DIR"/");
    }

    if(!dir.exists(DATA_DIR"/") ){
        dir.mkdir(DATA_DIR"/");
    }

    if(!dir.exists(WAVE_DIR"/") ){
        dir.mkdir(WAVE_DIR"/");
    }

    if(!dir.exists(FAVORITE_DIR"/") ){
        dir.mkdir(FAVORITE_DIR"/");
    }

    filepath = QString(WAVE_DIR"/" + filename);
    filepath_SD = QString("/mmc/sdcard/WaveForm/" + filename);
//    qDebug()<<filepath;
//    qDebug()<<filepath_SD;
}

void FileTools::saveDataFile()
{
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
        qDebug()<<file.fileName()<<"    "<<_data.length()<<" points";
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
        if(_mode == Double_Channel){
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

#if 0
        //拷贝到SD卡
        if(sqlcfg->get_para()->buzzer_on){
            if(file.copy(filepath_SD + ".DAT")){
                qDebug()<<"copy " + filename + ".DAT to SDCard succeed!";
            }
            else{
                qDebug()<<"copy " + filename + ".DAT to SDCard failed!";
            }
        }
#endif
        file.close();
    }
    else{
        qDebug()<<"file open failed!";
    }
}


void FileTools::saveCfgFile()
{
    QFile file;
    file.setFileName(filepath + ".CFG");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "wave1,,1999" << "\n";
    if(_mode == Double_Channel){
        out << "2,2A,0D" << "\n";
        out << "1,UA,,,V,1.000000,0.000000,0,-32768,32767,1.000000,1.000000,S" << "\n";
        out << "2,UB,,,V,1.000000,0.000000,0,-32768,32767,1.000000,1.000000,S" << "\n";
        out << sqlcfg->get_para()->freq_val << ".000000" << "\n";
        out << "1" << "\n";
        out << "1000000000," << _data.length()/2 << "\n";     //点数可变
    }
    else{
        out << "1,1A,0D" << "\n";
        out << "1,UA,,,V,1.000000,0.000000,0,-32768,32767,1.000000,1.000000,S" << "\n";
        out << sqlcfg->get_para()->freq_val << ".000000" << "\n";
        out << "1" << "\n";
        if(_mode == AA1 || _mode == AA2 || _mode == AE1 || _mode == AE2){
//            out << "400000," << _data.length()/2 << "\n";     //点数可变
            out << "4000," << _data.length() << "\n";     //点数可变
        }
        else{
//            out << "1000000000," << _data.length()/2 << "\n";     //点数可变
            out << "4000," << _data.length() << "\n";     //点数可变
        }
    }


    out << QDateTime::currentDateTime().toString() << "\n";
    out << QDateTime::currentDateTime().toString() << "\n";
    out << "BINARY" << "\n";
#if 0
    if(sqlcfg->get_para()->buzzer_on){
        if(file.copy(filepath_SD + ".CFG")){
            //        qDebug()<<"copy " + filename + ".CFG to SDCard succeed!";
        }
        else{
            qDebug()<<"copy " + filename + ".CFG to SDCard failed!";
        }
    }
#endif

    file.close();
}

//保存wav文件(至临时文件夹)
void FileTools::saveWavFile()
{
    int i, size;

    wfh1=(WaveFormat*)malloc(sizeof(WaveFormat));
    if (wfh1 == NULL) {
        return;
    }
    //    memset(wfh1,0,sizeof(wfh1));

    size = _data.length() * 2;

    strcpy(wfh1->RIFF,"RIFF");
    wfh1->Filesize = size + sizeof(WaveFormat);
    strcpy(wfh1->WavFlags,"WAVE");
    strcpy(wfh1->FMTFlags,"fmt ");
    wfh1->ByteFilter = 16;
    wfh1->Format = 1;
    wfh1->Channel = 1;
    if(_mode == AA1 || _mode == AA2 || _mode == AE1 || _mode == AE2 ){
//        wfh1->SampleFreq = 400000;
        wfh1->SampleFreq = 40000;   //临时测试时使用
    }
    else if( _mode == AA1_ENVELOPE || _mode == AA2_ENVELOPE || _mode == AE1_ENVELOPE || _mode == AE2_ENVELOPE){
        wfh1->SampleFreq = 40000;
    }

    wfh1->PCMBitSize = 16;
    wfh1->ByteFreq   = wfh1->PCMBitSize * wfh1->Channel * wfh1->SampleFreq / 8;     //640000
    wfh1->BlockAlign = wfh1->PCMBitSize * wfh1->Channel / 8;
    strcpy(wfh1->fact,"data");
    wfh1->Datasize = size;


    QFile file("out.wav");
    //    file.setFileName(filepath + ".wav");
//    file.setFileName("/tmp/out.wav");
    file.setFileName("/root/out.wav");

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

#if 1           //送包络线
    qint16 tmp;
    for(i=0;i<_data.length();i++){
        tmp = qint16(_data.at(i)*32);
        if(tmp < 0){
            tmp = 0x7fff;
        }
        out << tmp;
    }

#else
    for(i=0;i<_data.length();i++){
        out << 64 * qint16(_data.at(i)) ;
    }
#endif


    file.close();

    free (wfh1);
}

void FileTools::wavToMp3()
{
    QString program = "/root/lame";
    QStringList arguments1,arguments2;
//    arguments1 << "/tmp/out.wav" << filepath+".mp3";
//    arguments2 << "/tmp/out.wav" << filepath_SD +".mp3";
    arguments1 << "/root/out.wav" << filepath+".mp3";

    QProcess *myProcess1 = new QProcess;
    myProcess1->start(program, arguments1);
    QObject::connect(myProcess1,SIGNAL(finished(int)),myProcess1,SLOT(deleteLater()));

#if 0
    if(sqlcfg->get_para()->buzzer_on){
        QProcess *myProcess2 = new QProcess;
        myProcess2->start(program, arguments2);
        QObject::connect(myProcess2,SIGNAL(finished(int)),myProcess2,SLOT(deleteLater()));
    }
#endif
    qDebug()<<"mp3 file saved!";
}

void FileTools::wav_add_filter()
{
    QString program = "/root/check_envelop";
    QStringList arguments1;
    if(filepath.contains(".DAT")){
        arguments1 << filepath << "-k";
    }
    else{
        arguments1 << filepath + ".DAT" << "-k";
    }

//    qDebug()<<filepath + ".DAT";

    QProcess *myProcess1 = new QProcess;
    myProcess1->start(program, arguments1);
    QObject::connect(myProcess1,SIGNAL(finished(int)),myProcess1,SLOT(deleteLater()));
    qDebug()<<" wav_add_filter!";
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

//        system ("sync");
    }
}




