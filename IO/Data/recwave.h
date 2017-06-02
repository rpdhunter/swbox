#ifndef RECWAVE_H
#define RECWAVE_H

#include <QObject>
#include "data.h"
#include <QTime>
#include <QTimer>

#define GROUP_NUM_MAX           16              //组号最大值


class RecWave : public QObject
{
    Q_OBJECT
public:

    enum Status{
        Working,
        Waiting,
        Free
    } status;           //工作状态，只能由外部更改

    explicit RecWave(G_PARA *gdata, MODE mode, QObject *parent = 0);

    void recStart(int time = 10);   //time为录波时长

    void startWork();   //开始接收数据
    void work();        //接收数据

signals:
    void waveData(VectorList,MODE);

public slots:
    void AA_rec_end();          //AA超声录波结束


private:
    G_PARA *tdata;
    int groupNum;       //组号
    VectorList _data;    //录波数据
    int time;           //录波时长
    QTime time_start;         //时标
    QTimer *timer;       //计时器（超声波使用）
    MODE mode;          //工作模式
    int groupNum_Offset;         //组号偏置

};

#endif // RECWAVE_H
