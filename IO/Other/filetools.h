#ifndef FILETOOLS_H
#define FILETOOLS_H

#include <QRunnable>
#include <QObject>
#include "IO/Data/data.h"
#include <QProcess>
#include <QDir>


class FileTools : public QObject, public QRunnable
{
    Q_OBJECT
public:
    enum FileMode {
        Read,
        Write
    };


    FileTools(VectorList data, MODE mode, FileMode filemode);
    FileTools(QString str, FileMode filemode);

    QString get_filter_info();

    ~FileTools();

    void run();

signals:
    void readFinished(VectorList, MODE);

private slots:
    void deal_myProcess(QProcess::ProcessState state);

private:
    //wav文件头
    typedef struct WavFormatHeader
    {
        char RIFF[4];
        long Filesize;          //文件大小
        char WavFlags[4];
        char FMTFlags[4];
        int ByteFilter;
        short Format;
        short Channel;
        long SampleFreq;
        long ByteFreq;
        short BlockAlign;
        short PCMBitSize;
        char fact[4];
        long Datasize;
    }WaveFormat;

    VectorList _data;
    MODE _mode;
    WaveFormat *wfh1;
    QString filepath, filepath_SD;
    QString cfgfilepath;
    QString filename;
    FileMode _fileMode;

//    bool work;       //0为立刻执行

    void getReadFilePath(QString str);      //获得待读取文件的全路径
    void getWriteFilePath();                //给波形文件取名(包括路径)

    void saveDataFile();        //保存录波文件

    void saveCfgFile();         //生成配置文件
    void saveWavFile();         //保存声音文件
    void wavToMp3();
    void wav_add_filter();

    void spaceControl(QString str); //空间管理
    void rm(QDir dir, QString s);
    void mode_envelope_modify();        //根据实际情况修正mode
};

#endif // FILETOOLS_H
