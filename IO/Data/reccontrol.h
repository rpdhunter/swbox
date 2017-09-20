#ifndef RECCONTROL_H
#define RECCONTROL_H

#include <QObject>
#include "data.h"
#include "recwave.h"
#include <QTime>

class RecControl : public QObject
{
    Q_OBJECT
public:
    explicit RecControl(G_PARA *g_data, QObject *parent = 0);

    void recvRecData();     //接收录波数据
    MODE mode();
    void re_send_rec_continuous();
    int free_time();

signals:
    void waveData(VectorList,MODE);     //录波完成信号，并发送录波数据


public slots:
    void startRecWave(MODE mode , int time);       //启动录波，需要主函数建立连接'

private slots:
    void recWaveComplete(VectorList wave,MODE mode);
    void recContinuousComplete();

private:
    G_PARA *data;
    RecWave *tev1, *tev2, *hfct1, *hfct2, *aa;

    QTimer *timer_long;     //连续录波计时器
    QTimer *timer_interval;     //录波间隔计时器
    MODE _mode;
    int rec_double_flag;        //双通道录波的标志位（0为常态，2为双通道录波第一波分，1为双通道录波第二波分）
    VectorList rec_double;      //保存双通道录波的数据
    VectorList rec_continuous;  //保存双通道录波的数据

    int hfct_rec_times;

//    QTime base_time;
    QTime rec_continuous_free_time;       //记录连续录波中的空闲时间

};

#endif // RECCONTROL_H
