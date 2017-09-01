#include "fifodata.h"
#include <QThreadPool>

//建立数据连接，完成线程的初始化工作
FifoData::FifoData(G_PARA *g_data)
{

    tdata = g_data;     //与外部交互的数据指针

    fifocontrol = new FifoControl(g_data, this);
    reccontrol = new RecControl(g_data, this);    //完成录波、声音播放等功能
    connect(reccontrol,SIGNAL(waveData(VectorList,MODE)), this,SIGNAL(waveData(VectorList,MODE)) );
    connect(this,SIGNAL(playVoiceData(VectorList)), fifocontrol, SLOT(playVoiceData(VectorList)) );
    connect(this,SIGNAL(stop_play_voice()), fifocontrol, SLOT(stop_play_voice()) );
    connect(fifocontrol, SIGNAL(playVoiceProgress(int,int,bool)), this, SIGNAL(playVoiceProgress(int,int,bool)) );

    /* Start qthread */
    this->start();
}

void FifoData::startRecWave(MODE mode, int time)
{
    reccontrol->startRecWave(mode,time);
}

void FifoData::run(void)
{
    int ret = 0;
    int delay_time = DELAY_TIME_LONG;

    while (true) {
        fifocontrol->read_nomal_data();
        fifocontrol->read_prpd1_data();
        fifocontrol->read_prpd2_data();
        ret = fifocontrol->read_hfct1_data();
        ret = fifocontrol->read_hfct2_data();
        ret = fifocontrol->read_rec_data();

        if(tdata->recv_para_rec.recComplete > 0 && tdata->recv_para_rec.recComplete < 255){
//            qDebug()<<"fpga rec complete!";
//            tdata->set_send_para(sp_rec_start_tev1, 2);
            reccontrol->recvRecData();
        }
        if(ret > 200){
            delay_time = DELAY_TIME_SHORT;     //录波存在不同延迟控制可能性？
        }

        fifocontrol->playVoiceData();
//        qDebug()<<"delay_time:  "<< delay_time;
        //休眠
        msleep(delay_time);


/*
        if(hfct_mode && !isRecording){  //高频CT模式
            ret = recv_data (vbase_hfct1, tdata->recv_para_nomal.hfctData);       //接收数据
            send_para ();
            read_fpga_mode1 ();
            if (ret > 250) {
                tdata->recv_para_nomal.groupNum ++;
                if(tdata->recv_para_nomal.groupNum == 16){
                    tdata->recv_para_nomal.groupNum = 0;
                }
                msleep(2);
            }
            else {
                msleep(180);         //没数据，休息18ms
            }
        }
        else{       //普通模式和录波模式
            ret = recv_data (vbase_nomal, (unsigned int *)&(tdata->recv_para_nomal));       //接收数据
            //录波
            if (ret) {
    //                        qDebug()<<"recv recComplete = "<<tdata->recv_para.recComplete;      //打印收到的录播完成标志位
                if( tdata->recv_para_nomal.recComplete >0 && tdata->recv_para_nomal.recComplete <=15){       //录波完成可能值为0-15
                    recvRecData();  //开始接收数据(暂时禁用)
                }
            }

            send_para ();
            read_fpga(sp_read_fpga_nomal);
            if(playVoice){          //播放声音
                playVoiceData();
            }

            if(tdata->recv_para_nomal.recComplete >0 && tdata->recv_para_nomal.recComplete <=16){
                usleep(1);          //上传录波数据时，休眠时间较短
            }
            else{
                msleep(45);        //空闲时，休眠时间较长
            }
        }
        */
    }
    exit(0);        //跳出循环，理论上永远不会执行此句？
}

















