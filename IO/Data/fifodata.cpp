#include "fifodata.h"
#include <QThreadPool>
#include <QtDebug>
//#include <QTime>
#include "IO/Other/cpu.h"
#include "zynq.h"

//#define DELAY_TIME_LONG     200000
//#define DELAY_TIME_LONG     5000
//#define DELAY_TIME_MID      5000
#define DELAY_TIME_LONG     5000
#define DELAY_TIME_MID      500
#define DELAY_TIME_SHORT    50
//#define DELAY_TIME_SHORT    200000

//建立数据连接，完成线程的初始化工作
FifoData::FifoData(G_PARA *g_data)
{
    data = g_data;     //与外部交互的数据指针

    fifocontrol = new FifoControl(g_data, this);
    reccontrol = new RecControl(g_data, fifocontrol, this);    //完成录波、声音播放等功能

    timer_slow = new QTimer;
    timer_slow->start(100);
    connect(timer_slow,SIGNAL(timeout()),this,SLOT(do_slow()) );


    //录波功能
    connect(reccontrol,SIGNAL(waveData(VectorList,MODE)), this,SIGNAL(waveData(VectorList,MODE)) );
    connect(this,SIGNAL(startRecWave(MODE,int)), reccontrol,SLOT(startRecWave(MODE,int)) );

    //播放声音
    connect(this,SIGNAL(playVoiceData(VectorList)), fifocontrol, SLOT(playVoiceData(VectorList)) );
    connect(this,SIGNAL(stop_play_voice()), fifocontrol, SLOT(stop_play_voice()) );
    connect(fifocontrol, SIGNAL(playVoiceProgress(int,int,bool)), this, SIGNAL(playVoiceProgress(int,int,bool)) );

    //同步信号
    connect(this,SIGNAL(send_sync(qint64,qint64)),fifocontrol,SLOT(send_sync(qint64,qint64)) );

    /* Start qthread */
//    this->start();
//    this->setPriority(QThread::TimeCriticalPriority);
}

void FifoData::do_slow()
{
    read_slow = true;
}

void FifoData::run(void)
{
    pthread_t tid = pthread_self();
    set_thread_cpu(tid,CPU_1);

    int ret = 0;
    int delay_time = DELAY_TIME_LONG;

//    QTime t1 = QTime::currentTime(), t2;

    while (true) {
        //慢速数据(100ms更新一次)
        if(read_slow){
            //基本数据
            fifocontrol->read_fpga(sp_read_fpga_normal);
            fifocontrol->read_normal_data();

#if 0
            //PRPD数据
            fifocontrol->read_fpga(sp_read_fpga_prpd1);
            fifocontrol->read_prpd1_data();
            fifocontrol->read_fpga(sp_read_fpga_prpd2);
            fifocontrol->read_prpd2_data();
#endif
            read_slow = false;

#if 0
            //包络线数据
            if(data->recv_para_ae1.readComplete == 1){
                fifocontrol->read_fpga(sp_read_fpga_ae1);
                ret = fifocontrol->read_ae1_data();
    //            qDebug()<<"AE1 len = "<<ret<<"\trecComplete = "<<data->recv_para_ae1.recComplete
    //                      <<"\tgroup = "<<data->recv_para_ae1.groupNum;
                if(ret > 100){
                    data->recv_para_ae1.readComplete = 0;
                    emit ae1_update();
                    read_slow = true;
                }
            }

            if(data->recv_para_ae2.readComplete == 1){
                fifocontrol->read_fpga(sp_read_fpga_ae2);
                ret = fifocontrol->read_ae2_data();
                if(ret > 100){
//                    qDebug()<<"AE2 len = "<<ret<<"\trecComplete = "<<data->recv_para_ae2.recComplete
//                              <<"\tgroup = "<<data->recv_para_ae2.groupNum;
                    data->recv_para_ae2.readComplete = 0;
                    emit ae2_update();
                    read_slow = true;
                }
            }
#endif
        }



        //短脉冲数据
        if(sqlcfg->get_para()->menu_h1 != Disable){
            fifocontrol->read_fpga(sp_read_fpga_hfct1);
            fifocontrol->read_short1_data();
            if(data->recv_para_short1.empty == 0){
                emit short1_update();
            }
        }

        if(sqlcfg->get_para()->menu_h2 != Disable){
            fifocontrol->read_fpga(sp_read_fpga_hfct2);
            fifocontrol->read_short2_data();
            if(data->recv_para_short2.empty == 0){
                emit short2_update();
            }
        }


        //包络线数据
        if(data->recv_para_envelope1.readComplete == 1 ){
            fifocontrol->read_fpga(sp_read_fpga_ae1);
            ret = fifocontrol->read_ae1_data();
            if(ret > 100){
                data->recv_para_envelope1.readComplete = 0;
//                qDebug()<<"AE1 len = "<<ret<<"\trecComplete = "<<data->recv_para_ae1.recComplete
//                                          <<"\tgroup = "<<data->recv_para_ae1.groupNum;
                emit ae1_update();
            }
        }

        if(data->recv_para_envelope2.readComplete == 1){
            fifocontrol->read_fpga(sp_read_fpga_ae2);
            ret = fifocontrol->read_ae2_data();
            if(ret > 100){
                data->recv_para_envelope2.readComplete = 0;
//                                qDebug()<<"AE2 len = "<<ret<<"\trecComplete = "<<data->recv_para_ae2.recComplete
//                                                          <<"\tgroup = "<<data->recv_para_ae2.groupNum;
                emit ae2_update();
            }
        }


        //录波数据
        fifocontrol->read_fpga(sp_read_fpga_rec);
        usleep(delay_time);
        ret = fifocontrol->read_rec_data();
        if(data->recv_para_rec.recComplete > 0 && data->recv_para_rec.recComplete < 255){
//            qDebug()<<"data->recv_para_rec.recComplete:"<<data->recv_para_rec.recComplete;
            reccontrol->recv_rec_data();
        }

        //声音播放
        fifocontrol->play_voice_data();

        //发送数据
        fifocontrol->send_para();

        //延迟设置
        if(data->recv_para_rec.recComplete == 16 || data->recv_para_rec.recComplete == 32
                || data->recv_para_envelope1.recComplete == 2 || data->recv_para_envelope2.recComplete == 2 ){
            delay_time = DELAY_TIME_MID;
        }
        else if(reccontrol->mode() != Disable || data->recv_para_short1.empty == 0 || data->recv_para_short2.empty == 0
                || data->recv_para_envelope1.recComplete == 1 || data->recv_para_envelope2.recComplete == 1){
            delay_time = DELAY_TIME_SHORT;
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

















