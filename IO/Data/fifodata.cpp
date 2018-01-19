#include "fifodata.h"
#include <QThreadPool>
#include <QtDebug>
//#include <QTime>
#include "zynq.h"

//#define DELAY_TIME_LONG     200000
#define DELAY_TIME_LONG     5000
#define DELAY_TIME_MID      5000
#define DELAY_TIME_SHORT    50
//#define DELAY_TIME_SHORT    200000

//建立数据连接，完成线程的初始化工作
FifoData::FifoData(G_PARA *g_data)
{
    data = g_data;     //与外部交互的数据指针

    fifocontrol = new FifoControl(g_data, this);
    reccontrol = new RecControl(g_data, fifocontrol, this);    //完成录波、声音播放等功能

    timer_slow = new QTimer;
    timer_slow->start(200);
    connect(timer_slow,SIGNAL(timeout()),this,SLOT(do_slow()) );

    //录波功能
    connect(reccontrol,SIGNAL(waveData(VectorList,MODE)), this,SIGNAL(waveData(VectorList,MODE)) );
    connect(this,SIGNAL(startRecWave(MODE,int)), reccontrol,SLOT(startRecWave(MODE,int)) );

    //播放声音
    connect(this,SIGNAL(playVoiceData(VectorList)), fifocontrol, SLOT(playVoiceData(VectorList)) );
    connect(this,SIGNAL(stop_play_voice()), fifocontrol, SLOT(stop_play_voice()) );
    connect(fifocontrol, SIGNAL(playVoiceProgress(int,int,bool)), this, SIGNAL(playVoiceProgress(int,int,bool)) );

    //同步信号
    connect(this,SIGNAL(send_sync(uint)),fifocontrol,SLOT(send_sync(uint)) );

    /* Start qthread */
    this->start();
    this->setPriority(QThread::TimeCriticalPriority);
}

void FifoData::do_slow()
{
    read_slow = true;
}

void FifoData::run(void)
{
    int ret = 0;
    int delay_time = DELAY_TIME_LONG;

//    QTime t1 = QTime::currentTime(), t2;

    while (true) {
        //慢速数据
        if(read_slow){
            fifocontrol->read_fpga(sp_read_fpga_normal);
            fifocontrol->read_normal_data();
            fifocontrol->read_fpga(sp_read_fpga_prpd1);
            ret = fifocontrol->read_prpd1_data();
//            qDebug()<<"ret = "<<ret;
            fifocontrol->read_fpga(sp_read_fpga_prpd2);
            fifocontrol->read_prpd2_data();
            read_slow = false;
        }

        fifocontrol->read_fpga(sp_read_fpga_hfct1);
        fifocontrol->read_hfct1_data();
        if(data->recv_para_short1.empty == 0){
            emit short1_update();
        }
        fifocontrol->read_fpga(sp_read_fpga_hfct2);
        fifocontrol->read_hfct2_data();
        if(data->recv_para_short2.empty == 0){
            emit short2_update();
        }

        //录波数据

        fifocontrol->read_fpga(sp_read_fpga_rec);
//        usleep(10);
        usleep(delay_time);
        ret = fifocontrol->read_rec_data();

//        qDebug()<<"tdata->recv_para_rec.recComplete = "<<data->recv_para_rec.recComplete;
//        qDebug()<<"ret = "<<ret;

        if(data->recv_para_rec.recComplete > 0 && data->recv_para_rec.recComplete < 255){
//            qDebug()<<"tdata->recv_para_rec.recComplete = "<<data->recv_para_rec.recComplete;
            reccontrol->recv_rec_data();
        }

        fifocontrol->play_voice_data();

        fifocontrol->send_para();


        if(data->recv_para_rec.recComplete == 16){
            delay_time = DELAY_TIME_MID;
        }
        else if(reccontrol->mode() != Disable || data->recv_para_short1.empty == 0 || data->recv_para_short2.empty == 0){
            delay_time = DELAY_TIME_SHORT;
//            qDebug()<<"free time"<<reccontrol->free_time();
            if(reccontrol->free_time()>10){
                reccontrol->re_send_rec_continuous();
            }
        }
        else {
            delay_time = DELAY_TIME_LONG;
        }

//        t2 = QTime::currentTime();
//        int temp = t1.msecsTo(t2);
//        if(temp > 1000){
//            qDebug()<<"time-------------------------------------------------------->"<<temp;
//        }
//        t1 = t2;

//        qDebug()<<n << "\tdelay_time = "<< delay_time;

    }
    exit(0);        //跳出循环，理论上永远不会执行此句？
}

















