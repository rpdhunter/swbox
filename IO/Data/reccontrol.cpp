#include "reccontrol.h"
#include <QtDebug>

RecControl::RecControl(G_PARA *g_data, QObject *parent) : QObject(parent)
{
    //双通道录波
    rec_double_flag = 0;
    tdata = g_data;

    //开启3个录波通道
    tev1 = new RecWave(g_data, MODE::TEV1);
    tev2 = new RecWave(g_data, MODE::TEV2);
    hfct1 = new RecWave(g_data, MODE::HFCT1);
    hfct2 = new RecWave(g_data, MODE::HFCT2);
    aa = new RecWave(g_data, MODE::AA_Ultrasonic);

    connect(tev1,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));
    connect(tev2,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));
    connect(hfct1,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));
    connect(hfct2,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));
    connect(aa,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));
}

void RecControl::startRecWave(MODE mode, int time)
{
    switch (mode) {
    case TEV1:
        tev1->recStart(mode);
        break;
    case TEV2:
        tev2->recStart(mode);
    case HFCT1:
    case HFCT_CONTINUOUS:
        tev2->recStart(mode);
        break;
    case AA_Ultrasonic:
        aa->recStart(mode,time);
        break;
    default:
        break;
    }
    qDebug()<<"receive startRecWave signal! ... "<<mode;
}


void RecControl::recvRecData()
{
    //由于可能存在自发录播，且可能存在多路同事录播，需要根据FPGA反馈进行模式判断
    //多路同时录波，约定首先传送AD编号较小的通道数据
    //
    //下面算法基于以上生成
    MODE mode;
    int x = tdata->recv_para_rec.recComplete;
    if (x == 3) {
        rec_double_flag = 2;  //同步录波模式
    }

    if (x > 15 || x <= 0) {
        mode = Disable;
    }
    else if (x & 1) {
        mode = TEV1;   //TEV1
    }
    else if (x & 2) {
        mode = TEV2;   //TEV2
    }
    else if (x & 4) {
        mode = AA_Ultrasonic;   //AA超声
    }
    else{           //AD4暂时不用
        mode = Disable;
    }

    switch (mode) {
    case TEV1:
        if(tev2->status == RecWave::Working){
//            qDebug()<<"TEV2 Working --> Waiting";
            tev2->status = RecWave::Waiting;        //TEV1优先级高于TEV2 Working --> Waiting
        }
        if(tev1->status == RecWave::Free){
//            qDebug()<<"TEV1 Free --> Working";
            tev1->status = RecWave::Working;
//            qDebug()<<"start new rec!!!!!!!!! mode = TEV1!";
            tev1->startWork();
        }
        else{
            tev1->work();
        }
        break;

    case TEV2:
        switch (tev2->status) {
        case RecWave::Free:
            if(tev1->status == RecWave::Free){
                tev2->status = RecWave::Working;            //Free --> Working
//                qDebug()<<"TEV2 Free --> Working";
                tev2->startWork();
            }
            else{
                tev2->status = RecWave::Waiting;            //Free --> Waiting
//                qDebug()<<"TEV2 Free --> Waiting";
            }
            break;
        case RecWave::Working:      //继续上传
            //            qDebug()<<"TEV2 go on working";
            tev2->work();
            break;
        case RecWave::Waiting:      //改变状态
            if(tev1->status == RecWave::Free){
                tev2->status = RecWave::Working;            //Waiting --> Working
//                qDebug()<<"TEV2 Waiting --> Working";
                tev2->startWork();
            }
            break;
        default:
            break;
        }
        break;

    case AA_Ultrasonic:
        if(aa->status == RecWave::Free){
            aa->status = RecWave::Working;
            aa->startWork();
        }
        else{
            aa->work();
        }

        break;
    default:
        break;
    }
}

void RecControl::doRecWave()
{

}

//录播完成的处理
//使用了一次性执行的多线程处理模式——QRunnable
//经实际测试，线程能自动结束
void RecControl::recWaveComplete(VectorList wave, MODE mode)
{
    qDebug()<<"rec_double_flag= "<<rec_double_flag;
    if(rec_double_flag == 0){
        emit waveData(wave,mode);
    }
    else if(rec_double_flag == 2){
        rec_double = wave;
        rec_double_flag -- ;
    }
    else if(rec_double_flag == 1){
        rec_double.append(wave);
        rec_double_flag -- ;
        emit waveData(rec_double,Double_Channel);   //发送拼接好的双通道数据(保存操作在界面完成)

    }
}





















