#ifndef RECCONTROL_H
#define RECCONTROL_H

#include <QObject>
#include "data.h"
#include "recwave.h"

class RecControl : public QObject
{
    Q_OBJECT
public:
    explicit RecControl(G_PARA *g_data, QObject *parent = 0);

    void startRecWave(MODE mode , int time);       //启动录波，需要主函数建立连接

    void recvRecData();     //接收录波数据

    void doRecWave();       //处理录波接口

signals:
    void waveData(VectorList,MODE);     //录波完成信号，并发送录波数据

public slots:

private slots:
    void recWaveComplete(VectorList wave,MODE mode);


private:
    G_PARA *tdata;
    RecWave *tev1, *tev2, *hfct1, *hfct2, *aa;

    int rec_double_flag;    //双通道录波的标志位（0为常态，2为双通道录波第一波分，1为双通道录波第二波分）
    VectorList rec_double;  //保存双通道录波的数据


};

#endif // RECCONTROL_H
