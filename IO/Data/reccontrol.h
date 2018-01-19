#ifndef RECCONTROL_H
#define RECCONTROL_H

#include <QObject>
#include "data.h"
#include "recwave.h"
#include <QTime>
#include "fifocontrol.h"

class RecControl : public QObject
{
    Q_OBJECT
public:
    explicit RecControl(G_PARA *g_data, FifoControl *fifocontrol, QObject *parent = 0);

    void recv_rec_data();       //接收一次录波数据
    MODE mode();                //返回当前录波状态
    void re_send_rec_continuous();      //如果在连续录波状态下,发起一次连续录波
    int free_time();

signals:
    void waveData(VectorList,MODE);     //录波完成信号，并发送录波数据


public slots:
    void startRecWave(MODE mode , int time);       //启动录波

private slots:
    void rec_wave_complete(VectorList wave,MODE mode);
    void rec_continuous_complete();

private:
    G_PARA *data;
    RecWave *channel_h1, *channel_h2, *channel_l1, *channel_l2;

    FifoControl *fifocontrol;

    QTimer *timer_long;     //连续录波计时器
    QTimer *timer_interval;     //录波间隔计时器
    MODE _mode;
    int rec_double_flag;        //双通道录波的标志位（0为常态，2为双通道录波第一波分，1为双通道录波第二波分）
    VectorList rec_double;      //保存双通道录波的数据
    VectorList rec_continuous;  //保存双通道录波的数据

    int hfct_rec_times;

//    QTime base_time;
    QTime rec_continuous_free_time;       //记录连续录波中的空闲时间


    void fft_test(VectorList inputlist);        //fft测试程序

};

#endif // RECCONTROL_H
