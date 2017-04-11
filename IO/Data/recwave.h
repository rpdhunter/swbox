#ifndef RECWAVE_H
#define RECWAVE_H

#include <QObject>
#include <QTime>
#include <QTimer>
#include "data.h"

#define GROUP_NUM_MAX           16              //组号最大值

class RecWave : public QObject
{
    Q_OBJECT
public:
    explicit RecWave(G_PARA *gdata, MODE mode, QObject *parent = 0);

    void recStart();

    void startWork();   //开始接收数据
    void work();        //接收数据

    enum Status{
        Working,
        Pending,
        Free
    } status;           //工作状态，只能由外部更改

signals:
    void waveData(VectorList,MODE);

public slots:
    void AA_rec_end();          //AA超声录波结束
    void saveWaveToFile();  //保存录波文件


private:
    int num;
    G_PARA *tdata;
    int groupNum;       //组号
    VectorList _data;    //录波数据
    QTime time;         //时标
    QTimer *timer;       //计时器（超声波使用）
    MODE mode;          //工作模式


    QString getFilePath();     //返回保存波形文件的目录
    void saveCfgFile(QString str);     //生成配置文件

};

#endif // RECWAVE_H
