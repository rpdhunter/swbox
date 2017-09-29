#include "reccontrol.h"
#include <QtDebug>
#include "IO/Other/filetools.h"
#include <QThreadPool>
#include "IO/SqlCfg/sqlcfg.h"

RecControl::RecControl(G_PARA *g_data, QObject *parent) : QObject(parent)
{
    //双通道录波
    rec_double_flag = 0;
    data = g_data;
    _mode = Disable;
    timer_interval = NULL;

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

    timer_long = new QTimer;
    timer_long->setSingleShot(true);
    connect(timer_long,SIGNAL(timeout()), this, SLOT(recContinuousComplete()) );


}

//从UI发起的的录波入口
void RecControl::startRecWave(MODE mode, int time)
{
    if(timer_long->isActive() || rec_double_flag != 0){
        qDebug()<<"rec channel is busy now! exit!";
        return;
    }

    this->_mode = mode;
    switch (mode) {
    case TEV1:
        tev1->recStart(mode);
        break;
    case TEV2:
        tev2->recStart(mode);
        break;
    case HFCT1:
        hfct1->recStart(mode);
        break;
    case HFCT2:
        hfct2->recStart(mode);
        break;
    case HFCT1_CONTINUOUS:
        hfct1->recStart(HFCT1);
        timer_long->start(time * 1000);
        rec_continuous.clear();
        break;
    case HFCT2_CONTINUOUS:
        hfct2->recStart(HFCT2);
        timer_long->start(time * 1000);
        rec_continuous.clear();
        break;
    case AA_Ultrasonic:
        aa->recStart(mode,time);
        break;
    default:
        break;
    }
    qDebug()<<"receive startRecWave signal! ... "<<mode;
}

/*****************************************************************
 * 接收从FPGA返回的录波完成信号
 * 由于可能存在自发录播，且可能存在多路同事录播，需要根据FPGA反馈进行模式判断
 * 多路同时录波，约定首先传送AD编号较小的通道数据
 * TEV1 ---- 0000 0001(1)
 * TEV2 ---- 0000 0010(2)
 * AA   ---- 0000 0100(4)
 * HFCT1---- 0100 0000(64)
 * HFCT2---- 1000 0000(128)
 * 下面算法基于以上生成
 * ***************************************************************/
void RecControl::recvRecData()
{
    MODE mode;
    int x = data->recv_para_rec.recComplete;

//    qDebug()<<"recComplete = "<< x;
    if (x == 3 || x == 65 || x == 66  || x == 129 || x == 130 || x == 192) {
        rec_double_flag = 2;  //同步录波模式
        qDebug()<<"rece rec_double_signal";
        this->_mode = Double_Channel;
    }

    if (x > 255 || x <= 0) {
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
    else if (x & 64){
        mode = HFCT1;
    }
    else if (x & 128){
        mode = HFCT2;
    }
    else{           //AD4暂时不用
        mode = Disable;
    }

    //如果是自动录波,需要改变模式
    if(this->_mode == Disable && mode != Disable){
        if(this->_mode != HFCT1_CONTINUOUS && this->_mode != HFCT2_CONTINUOUS && this->_mode != Double_Channel){     //自动录波且非连续录波
            if(timer_interval == NULL){
                timer_interval = new QTimer;
                timer_interval->setSingleShot(true);
                qDebug()<<"init timer_interval";
            }
            if(timer_interval->remainingTime() > 0){
                qDebug()<<"tringer the interval! remain timer:"<<timer_interval->remainingTime();
                data->set_send_para(sp_rec_start_tev1, 2);
                data->set_send_para(sp_rec_start_tev1, 0);
                data->set_send_para(sp_rec_start_tev2, 2);
                data->set_send_para(sp_rec_start_tev2, 0);
                data->set_send_para(sp_rec_start_hfct1, 2);
                data->set_send_para(sp_rec_start_hfct1, 0);
                data->set_send_para(sp_rec_start_hfct2, 2);
                data->set_send_para(sp_rec_start_hfct2, 0);
                return;
            }
            else{
                timer_interval->start(sqlcfg->get_para()->auto_rec_interval * 1000);    //开始设置间隔
                this->_mode = mode;
            }
        }
    }

    switch (mode) {
    case TEV1:
        if(tev1->status == RecWave::Free){
            tev1->status = RecWave::Working;
            tev1->startWork();
        }
        else{
            tev1->work();
        }
        break;
    case TEV2:
        if(tev2->status == RecWave::Free){
            tev2->status = RecWave::Working;
            tev2->startWork();
        }
        else{
            tev2->work();
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
    case HFCT1:
        if(hfct1->status == RecWave::Free){
            hfct1->status = RecWave::Working;
            hfct1->startWork();
        }
        else{
            hfct1->work();
        }
        break;
    case HFCT2:
        if(hfct2->status == RecWave::Free){
            hfct2->status = RecWave::Working;
            hfct2->startWork();
        }
        else{
            hfct2->work();
        }
        break;
    default:
        break;
    }

    /*    switch (mode) {
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
    */
}

MODE RecControl::mode()
{
    return _mode;
}

void RecControl::re_send_rec_continuous()
{
    rec_continuous_free_time = QTime::currentTime();
    hfct_rec_times = 0;
    if(_mode == HFCT1_CONTINUOUS){
        hfct1->recStart(HFCT1);
    }
    else if(_mode == HFCT2_CONTINUOUS){
        hfct2->recStart(HFCT2);
    }
}

int RecControl::free_time()
{
//    return hfct_rec_times;
    return rec_continuous_free_time.msecsTo(QTime::currentTime() );
}

//录播完成的处理
//使用了一次性执行的多线程处理模式——QRunnable
//经实际测试，线程能自动结束
void RecControl::recWaveComplete(VectorList wave, MODE mode)
{
//    qDebug()<<"rec_double_flag= "<<rec_double_flag;

    if(rec_double_flag == 2){
        rec_double = wave;
        rec_double_flag -- ;
    }
    else if(rec_double_flag == 1){
        rec_double.append(wave);
        rec_double_flag -- ;
        emit waveData(rec_double,Double_Channel);   //发送拼接好的双通道数据(保存操作在界面完成)
        this->_mode = Disable;
//        FileTools *filetools = new FileTools(rec_double,Double_Channel);      //开一个线程，为了不影响数据接口性能
//        QThreadPool::globalInstance()->start(filetools);
    }
    else if(timer_long->isActive()){
        re_send_rec_continuous();
        rec_continuous.append(wave);
    }
    else if(this->_mode == Disable){
//        this->_mode = mode;
    }
    else if(this->_mode != Disable){
        emit waveData(wave,mode);
        this->_mode = Disable;
        FileTools *filetools = new FileTools(wave,mode,FileTools::Write);      //开一个线程，为了不影响数据接口性能
        QThreadPool::globalInstance()->start(filetools);
    }
}

void RecControl::recContinuousComplete()
{
    emit waveData(rec_continuous, _mode);
    qDebug()<<"rec continuous complete, times ============================================================== "<< rec_continuous.length()/4000;
    FileTools *filetools = new FileTools(rec_continuous,_mode,FileTools::Write);      //开一个线程，为了不影响数据接口性能
    _mode = Disable;         //Disable为去掉最后一次连续录波数据
    QThreadPool::globalInstance()->start(filetools);
}





















