#include "fifodata.h"
#include <QThreadPool>
#include <QtDebug>
#include "IO/Other/cpu.h"
#include "zynq.h"

//#define DELAY_TIME_LONG     2000        //必须保证包络线数据每秒能接收312.5次(<3200)
#define DELAY_TIME_LONG     5000
#define DELAY_TIME_MID      500
#define DELAY_TIME_SHORT    50

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
    connect(this,SIGNAL(send_sync(qint64,qint64)),fifocontrol,SLOT(send_sync(qint64,qint64)), Qt::DirectConnection);

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

    data->recv_para_short1.data_flag = 1;           //初始化这两个值,目的是降低
    data->recv_para_short2.data_flag = 1;

    while (true) {
        //慢速数据(100ms更新一次)        
        if(read_slow){
            //基本数据
            fifocontrol->read_fpga(sp_read_fpga_normal);
            ret = fifocontrol->read_normal_data();
//            qDebug()<<"read_normal_data len =" << ret;           //每次收到10位

//            unsigned int *temp;
//            temp = (unsigned int *)&(data->recv_para_normal);
//            for (int i = 0; i < ret; ++i) {
//                printf("%08x \t", temp[i]);
//                if((i+1) % 25 == 0){
//                    printf("\n");
//                }
//            }
//            printf("\n");

            read_slow = false;
        }

        //短脉冲数据
        if(sqlcfg->get_para()->menu_h1 != Disable){
            fifocontrol->read_fpga(sp_read_fpga_short1);
            fifocontrol->read_short1_data();
            if(data->recv_para_short1.data_flag == 0){
                emit short1_update();
            }
        }

        if(sqlcfg->get_para()->menu_h2 != Disable){
            fifocontrol->read_fpga(sp_read_fpga_short2);
            fifocontrol->read_short2_data();
            if(data->recv_para_short2.data_flag == 0){
                emit short2_update();
            }
        }


        //包络线数据
#if 0
        if(data->recv_para_envelope1.readComplete == 1 ){
            fifocontrol->read_fpga(sp_read_fpga_envelope1);
            ret = fifocontrol->read_ae1_data();
//            qDebug()<<"recv_para_envelope1 len =" << ret;           //每次收到134位
//            unsigned int *temp;
//            temp = (unsigned int *)&(data->recv_para_envelope1);
//            for (int i = 0; i < ret; ++i) {
//                printf("%4x \t", temp[i]);
//                if((i+1) % 25 == 0){
//                    printf("\n");
//                }
//            }
//            printf("\n");
            if(ret > 100){
//                qDebug()<<"recv_para_envelope1 len =" << ret;           //每次收到134位
                data->recv_para_envelope1.readComplete = 0;
                emit envelope1_update();
            }
        }

        if(data->recv_para_envelope2.readComplete == 1){
            fifocontrol->read_fpga(sp_read_fpga_envelope2);
            ret = fifocontrol->read_ae2_data();
            if(ret > 100){
                data->recv_para_envelope2.readComplete = 0;
                emit envelope2_update();
            }
        }

#endif
        fifocontrol->read_fpga(sp_read_fpga_envelope1);
        VectorList envelope1_data(134);                 //实际使用134,如果异常状况数据变多,越界也不会造成崩溃
        ret = fifocontrol->read_ae1_data(envelope1_data.data());
        if(ret == 134){
//            qDebug()<<"send";
            emit envelope1_update(envelope1_data);      //发送数据
        }

        fifocontrol->read_fpga(sp_read_fpga_envelope2);
        VectorList envelope2_data(134);                 //实际使用134,如果异常状况数据变多,越界也不会造成崩溃
        ret = fifocontrol->read_ae2_data(envelope2_data.data());
        if(ret == 134){
            emit envelope2_update(envelope2_data);      //发送数据
        }


        //录波数据
        fifocontrol->read_fpga(sp_read_fpga_rec);
        usleep(delay_time);
        ret = fifocontrol->read_rec_data();
//        qDebug()<<"ret = "<<ret;
        if(ret > 2 && data->recv_para_rec.recComplete > 0 && data->recv_para_rec.recComplete < 255){
//            qDebug()<<"data->recv_para_rec.recComplete:"<<data->recv_para_rec.recComplete;
//            qDebug()<<"ret = "<<ret;
            reccontrol->recv_rec_data();
        }

        //声音播放
        fifocontrol->play_voice_data();

        //发送数据
        fifocontrol->send_para();

        //延迟设置
        if(data->recv_para_rec.recComplete == 16 || data->recv_para_rec.recComplete == 32
                || envelope1_data.at(0) == 2 || envelope2_data.at(0) == 2 ){
            delay_time = DELAY_TIME_MID;
        }
        else if(reccontrol->mode() != Disable || data->recv_para_short1.data_flag == 0 || data->recv_para_short2.data_flag == 0
                || envelope1_data.at(0) == 1 || envelope2_data.at(0) == 1){
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
//        if(temp > 5){
//            qDebug()<<"time-------------------------------------------------------->"<<temp << "ms" << QTime::currentTime().toString("ss-zzz");
//            qDebug() << "\tdelay_time = "<< delay_time;
//        }
//        t1 = t2;

//        qDebug() << "\tdelay_time = "<< delay_time;

    }
    exit(0);        //跳出循环，理论上永远不会执行此句？
}

















