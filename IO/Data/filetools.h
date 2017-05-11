#ifndef FILETOOLS_H
#define FILETOOLS_H

#include <QRunnable>
#include "data.h"


class FileTools : public QRunnable
{
public:
    FileTools(VectorList data, MODE mode);

    ~FileTools();

    void run();

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
    WavFormatHeader *wfh1;
    QString filepath;
    QString filename;

//    bool work;       //0为立刻执行

    void saveDataFile();        //保存录波文件
    QString getFilePath();      //返回保存波形文件的目录
    void saveCfgFile();         //生成配置文件
    void saveWavFile();         //保存声音文件
    void wavToMp3();

    void spaceControl(QString str); //空间管理
};

#endif // FILETOOLS_H
