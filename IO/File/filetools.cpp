#include "filetools.h"
#include <QtDebug>
#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include "spacecontrol.h"

#include "IO/SqlCfg/sqlcfg.h"
#include "Gui/Common/common.h"
#include "Algorithm/fir.h"


//还没有开启冲突控制
FileTools::FileTools(VectorList data, MODE mode, FileMode filemode)
{
    _data = data;
    _mode = mode;
    _fileMode = filemode;
    qRegisterMetaType<QProcess::ProcessState>("QProcess::ProcessState");
    mode_envelope_modify();
}

FileTools::FileTools(QString str,FileTools::FileMode filemode)
{
    getReadFilePath(str);
    _fileMode = filemode;
}

void FileTools::get_file_info(MODE &mod, QDateTime &d_t, double &gain, double &threshold, bool &wavelet, QString &filter_info)
{
    if(!cfgfilepath.isEmpty()){
        QFile file;
        file.setFileName(cfgfilepath);

        if (!file.open(QIODevice::ReadOnly)){
            qDebug()<<"file open failed!";
            return ;
        }

        QTextStream in(&file);
        QString tmpstr;

        //这里有个隐患，如果录波文件损坏，程序将卡死
        while (!in.atEnd()) {
            tmpstr = in.readLine();
            if( tmpstr.contains("Mode,")){          //模式
                tmpstr.remove("Mode,");
                mod = Common::string_to_mode(tmpstr);
            }
            if( tmpstr.contains("DateTime,")){      //时间日期
                tmpstr.remove("DateTime,");
                d_t = QDateTime::fromString(tmpstr);
            }
            if( tmpstr.contains("Wavelet,")){       //小波
                tmpstr.remove("Wavelet,");
                wavelet = tmpstr.toInt();
            }
            if( tmpstr.contains("Gain,")){          //增益
                tmpstr.remove("Gain,");
                gain = tmpstr.toDouble();
            }
            if( tmpstr.contains("Threshold,")){     //阈值
                tmpstr.remove("Threshold,");
                threshold = tmpstr.toDouble();
            }
            if( tmpstr.contains("PB,")){            //通带宽度
                tmpstr.remove("PB,");
                filter_info =  tmpstr;
            }
        }
    }
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
        Common::create_hard_link(filepath+".DAT",filename+".DAT");
        Common::create_hard_link(filepath+".CFG",filename+".CFG");
//        qDebug()<<"filepath:"<<filepath<<"\filename:"<<filename;

        if(_mode == AA1 || _mode == AA2 || _mode == AE1 || _mode == AE2
                || _mode == AA_ENVELOPE1 || _mode == AA_ENVELOPE2 || _mode == AE_ENVELOPE1 || _mode == AE_ENVELOPE2){
            saveWavFile();      //生成声音文件
            wavToMp3();         //mp3转换

//            wav_add_filter();       //TEST
        }

        //空间管理
        spaceControl(DIR_WAVE"/");              //内存空间管理
//        spaceControl("/mmc/sdcard/WaveForm/");  //SD卡空间管理

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

        //这里有个隐患，如果录波文件损坏，程序将卡死
        while (!in.atEnd()) {
            in >> t1 >> t2 >> v;
            _data.append(v);
            if( _mode == Double_Channel){
                in >> v;
                _data.append(v);
            }
        }

        emit readFinished(_data, _mode);

        //以下为测试代码
#if 0
        if(_mode == AA1 || _mode == AA2 || _mode == AE1 || _mode == AE2         //TEST
                || _mode == AA1_ENVELOPE || _mode == AA2_ENVELOPE || _mode == AE1_ENVELOPE || _mode == AE2_ENVELOPE){
            if(filepath.contains("_env")){
                saveWavFile();      //生成声音文件
                wavToMp3();         //mp3转换
            }
        }

        qDebug()<<"trans hfct:"<<filepath << Common::MODE_toString(_mode);
        if(_mode == HFCT1 || _mode == HFCT2){
            filepath.remove(".DAT");
            filepath.append("_trans");
            Fir fir;
            qDebug()<<_data.mid(0,10);
            _data = fir.set_filter(_data,hp_2M);
            qDebug()<<_data.mid(0,10);
            saveDataFile();         //保存数据文件
            saveCfgFile();          //生成对应的配置文件
        }
#endif
    }
}

//待完成
void FileTools::deal_myProcess(QProcess::ProcessState state)
{
//    switch (state) {
//    case QProcess::NotRunning:
//        qDebug()<<"QProcess::NotRunning";
//        break;
//    case QProcess::Starting:
//        qDebug()<<"QProcess::Starting";
//        break;
//    case QProcess::Running:
//        qDebug()<<"QProcess::Running";
//        break;
//    default:
//        break;
//    }
//    qDebug()<<"mp3 link created!";
//    Common::create_hard_link(filepath+".mp3",filename+".mp3");
}


void FileTools::mode_envelope_modify()
{
    if(_mode == AA1 && sqlcfg->get_para()->aa1_sql.envelope == true){
        _mode = AA_ENVELOPE1;
    }
    else if(_mode == AA2 && sqlcfg->get_para()->aa2_sql.envelope == true){
        _mode = AA_ENVELOPE2;
    }
    else if(_mode == AE1 && sqlcfg->get_para()->ae1_sql.envelope == true){
        _mode = AE_ENVELOPE1;
    }
    else if(_mode == AE2 && sqlcfg->get_para()->ae2_sql.envelope == true){
        _mode = AE_ENVELOPE2;
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
        if(str.contains("ENVELOPE")){
            _mode = AA_ENVELOPE1;
        }
        else{
            _mode = AA1;
        }
    }
    else if(str.contains("AA2_")){
        if(str.contains("ENVELOPE")){
            _mode = AA_ENVELOPE2;
        }
        else{
            _mode = AA2;
        }
    }
    else if(str.contains("AE1_")){
        if(str.contains("ENVELOPE")){
            _mode = AE_ENVELOPE1;
        }
        else{
            _mode = AE1;
        }
    }
    else if(str.contains("AE2_")){
        if(str.contains("ENVELOPE")){
            _mode = AE_ENVELOPE2;
        }
        else{
            _mode = AE2;
        }
    }
    else if(str.contains("Double_")){
        _mode = Double_Channel;
    }

    if(str.contains(QString("☆") )){
        filepath = QString(DIR_FAVORITE"/" + str.remove(QString("☆") ) + ".DAT");        //收藏夹
        cfgfilepath = QString(DIR_FAVORITE"/" + str.remove(QString("☆") ) + ".CFG");
    }
    else{
        filepath = QString(DIR_WAVE"/"+str+".DAT");
        cfgfilepath = QString(DIR_WAVE"/"+str+".CFG");          //保存一个CFG路径
    }
}

//实现优先在SD卡建立波形存储文件夹
//如果SD卡未插入，则在内存当前文件夹下建立波形文件夹
//返回波形文件夹路径
void FileTools::getWriteFilePath()
{
    filename = QString("%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz"));
    filename.prepend(Common::mode_to_string(_mode).append("_") );

    //文件夹操作
    Common::mk_dir(DIR_USB);
    Common::mk_dir(DIR_DATA);
    Common::mk_dir(DIR_WAVE);
    Common::mk_dir(DIR_FAVORITE);

    filepath = QString(DIR_WAVE"/" + filename);
}

void FileTools::saveDataFile()
{
    QFile file;
    bool flag;

    //保存文本文件
#if 0
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
#endif

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

//        qDebug()<<tr("save %1 success").arg(filepath + ".DAT");

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
            out << "4000," << _data.length() << "\n";     //点数可变
        }
        else{
            out << "4000," << _data.length() << "\n";     //点数可变
        }
    }


    out << QDateTime::currentDateTime().toString() << "\n";     //时间日期
    out << QDateTime::currentDateTime().toString() << "\n";     //时间日期(必须存两次,才能用南瑞软件打开,这之前的不要动)
    out << "BINARY" << "\n";                            //二进制
    //保存模式信息
    out << "Mode,"<< Common::mode_to_string(_mode)<< "\n";
    //保存时间日期
    out << "DateTime,"<< QDateTime::currentDateTime().toString()<< "\n";
    //保存FIR滤波器信息
    out << "PB,";
    out << Common::filter_to_string(Common::channel_sql(_mode)->filter_hp) << "-"
        << Common::filter_to_string(Common::channel_sql(_mode)->filter_lp) << "\n";
    //小波滤波
    out << "Wavelet,"<< Common::channel_sql(_mode)->filter_wavelet << "\n";
    //保存通道增益
    out << "Gain,"<< Common::channel_sql(_mode)->gain << "\n";
    //保存通道阈值
    out << "Threshold,"<< Common::channel_sql(_mode)->fpga_threshold << "\n";

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
    else if( _mode == AA_ENVELOPE1 || _mode == AA_ENVELOPE2 || _mode == AE_ENVELOPE1 || _mode == AE_ENVELOPE2){
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

    connect(myProcess1,SIGNAL(stateChanged(QProcess::ProcessState)),this,SLOT(deal_myProcess(QProcess::ProcessState)));
//    QObject::connect(myProcess1,SIGNAL(finished(int)),myProcess1,SLOT(deleteLater()));
//    QObject::connect(myProcess1,SIGNAL(finished(int)),this,SLOT(deal_myProcess()));     //建立mp3资产软连接
    myProcess1->start(program, arguments1);



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
    QStringList filters;
    filters << "*.DAT" << "*.dat";
    QStringList list = dir.entryList(filters,QDir::Files,QDir::Time);

//    qDebug()<<"max_rec_num:"<<sqlcfg->get_para()->max_rec_num << "\tcurrent"<<list.length();

    //按文件数目删除
    if(list.length()>sqlcfg->get_para()->max_rec_num){
        for(int i=0;i<12;i++){          //删除12个文件
            QString s = list.at(list.length() - i - 1);
            rm(dir,s);
            s.remove(".DAT");
            rm(dir, s + ".mp3");
            rm(dir, s + ".CFG");
            rm(dir, s + ".txt");
        }
    }

    //按空间大小删除
//    qDebug()<<"DIR_WAVE file size:"<<Common::dirFileSize(DIR_WAVE);
    SpaceControl::file_bySize(str, 2e9);         //大于2G,开删

}

void FileTools::rm(QDir dir, QString s)
{
    if(dir.remove(s)){
//        qDebug()<<"remove file "<<dir.path() + '/' + s<<" succeed!";
    }
    else{
//        qDebug()<<"remove file "<<dir.path() + '/'+ s<<" failed!";
    }
}




