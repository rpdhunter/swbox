#ifndef FIFODATA_H
#define FIFODATA_H

#include <QThread>
#include "zynq.h"
#include "data.h"
#include "IO/File/filetools.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "reccontrol.h"
#include "fifocontrol.h"


//用于从FPGA读取数据
//这是一个多线程函数，开启一个新线程，在不影响主界面操作的同时持续监听数据输入
//数据读取单位是G_PARA
class FifoData : public QThread
{
    Q_OBJECT
public:
    explicit FifoData(G_PARA *g_data);

public slots:


signals:
    void waveData(VectorList,MODE);     //录波完成信号，并发送录波数据
    void startRecWave(MODE mode , int time);       //启动录波，需要主函数建立连接

    void playVoiceData(VectorList wave);            //向FPGA发送声音数据
    void stop_play_voice();
    void playVoiceProgress(int cur, int all, bool);        //返回播放声音的实时进度，前2个参数是播放进度，后一个是是否播完，0未播完，1播完

    void send_sync(qint64,qint64);          //发送同步时间（秒，微秒）

    void short1_update();       //高频1通道数据更新
    void short2_update();       //高频1通道数据更新
//    void envelope1_update();       //包络线数据更新（低频1）
    void envelope1_update(VectorList);  //包络线数据更新（低频1）
    void envelope2_update(VectorList);  //包络线数据更新（低频2）
//    void envelope2_update();       //包络线数据更新（低频2）

private slots:
    void do_slow();

private:
    G_PARA * data;
    RecControl *reccontrol;   //完成数据接口的杂项控制
    FifoControl *fifocontrol;
    bool read_slow;

    QTimer *timer_slow;

protected:
    void run(void);

};

#endif // RECVFIFODATA_H
