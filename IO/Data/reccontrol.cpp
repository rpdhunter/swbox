#include "reccontrol.h"
#include <QtDebug>
#include "IO/File/filetools.h"
#include <QThreadPool>
#include "IO/SqlCfg/sqlcfg.h"
#include "Algorithm/fft.h"
#include "Gui/Common/common.h"
#include "Algorithm/fir.h"
#include "Algorithm/Wavelet/wavelet.h"

RecControl::RecControl(G_PARA *g_data, FifoControl *fifocontrol, QObject *parent) : QObject(parent)
{
    //双通道录波
    rec_double_flag = 0;
    data = g_data;
    _mode = Disable;
    timer_interval = NULL;
    this->fifocontrol = fifocontrol;

    //开启4个录波通道
    channel_h1 = new RecWave(g_data, CHANNEL_H1);
    channel_h2 = new RecWave(g_data, CHANNEL_H2);
    channel_l1 = new RecWave(g_data, CHANNEL_L1);
    channel_l2 = new RecWave(g_data, CHANNEL_L2);

    //四个通道录播完成处理
    connect(channel_h1,SIGNAL(waveData(VectorList,MODE)),this,SLOT(rec_wave_complete(VectorList,MODE)));
    connect(channel_h2,SIGNAL(waveData(VectorList,MODE)),this,SLOT(rec_wave_complete(VectorList,MODE)));
    connect(channel_l1,SIGNAL(waveData(VectorList,MODE)),this,SLOT(rec_wave_complete(VectorList,MODE)));
    connect(channel_l2,SIGNAL(waveData(VectorList,MODE)),this,SLOT(rec_wave_complete(VectorList,MODE)));

    //处理连续录波计时
    timer_long = new QTimer;
    timer_long->setSingleShot(true);
    connect(timer_long,SIGNAL(timeout()), this, SLOT(rec_continuous_complete()) );
}

//从UI发起的的录波入口
void RecControl::startRecWave(MODE mode, int time)
{
    if(timer_long->isActive() || rec_double_flag != 0){
        qDebug()<<"rec channel is busy now! exit!";
        return;
    }

    data->set_send_para(sp_rec_on,1);           //开启录波

    this->_mode = mode;

    switch (this->_mode) {
    case TEV1:
        channel_h1->recStart();
        if(time > 0){
            this->_mode = TEV_CONTINUOUS1;
            timer_long->start(time * 1000);
            rec_continuous.clear();
        }
        break;
    case HFCT1:
        channel_h1->recStart();
        if(time > 0){
            this->_mode = HFCT_CONTINUOUS1;
            timer_long->start(time * 1000);
            rec_continuous.clear();
        }
        break;
    case UHF1:
        channel_h1->recStart();
        if(time > 0){
            this->_mode = UHF_CONTINUOUS1;
            timer_long->start(time * 1000);
            rec_continuous.clear();
        }
        break;
    case TEV2:
        channel_h2->recStart();
        if(time > 0){
            this->_mode = TEV_CONTINUOUS2;
            timer_long->start(time * 1000);
            rec_continuous.clear();
        }
        break;
    case HFCT2:
        channel_h2->recStart();
        if(time > 0){
            this->_mode = HFCT_CONTINUOUS2;
            timer_long->start(time * 1000);
            rec_continuous.clear();
        }
        break;
    case UHF2:
        channel_h2->recStart();
        if(time > 0){
            this->_mode = UHF_CONTINUOUS2;
            timer_long->start(time * 1000);
            rec_continuous.clear();
        }
        break;
    case AA1:
    case AE1:
        channel_l1->recStart(time);
        break;
    case AA2:
    case AE2:
        channel_l2->recStart(time);
        break;
    default:
        break;
    }
    qDebug()<<"receive startRecWave signal! ... "<< Common::mode_to_string(this->_mode);
}

/*****************************************************************
 * 接收从FPGA返回的录波完成信号
 * 由于可能存在自发录播，且可能存在多路同事录播，需要根据FPGA反馈进行模式判断
 * 多路同时录波，约定首先传送AD编号较小的通道数据
 * H1  ---- 0000 0001(1)
 * H2  ---- 0000 0010(2)
 * L1  ---- 0000 0100(4)
 * L2  ---- 0000 1000(8)
 * 下面算法基于以上生成
 * ***************************************************************/
void RecControl::recv_rec_data()
{
    MODE mode;
    int x = data->recv_para_rec.recComplete;

    if (x == 3 || x == 65 || x == 66  || x == 129 || x == 130 || x == 192) {
        rec_double_flag = 2;  //同步录波模式
        qDebug()<<"rece rec_double_signal, x = " << x;
        this->_mode = Double_Channel;
    }

//    qDebug()<<"data->recv_para_rec.recComplete = "<<data->recv_para_rec.recComplete;

    if (x > 255 || x <= 0) {
        mode = Disable;
    }
    else if (x & 1) {
        mode = (MODE)sqlcfg->get_para()->menu_h1;
    }
    else if (x & 2) {
        mode = (MODE)sqlcfg->get_para()->menu_h2;
    }
    else if (x & 4) {
        mode = (MODE)sqlcfg->get_para()->menu_l1;
    }
    else if (x & 8) {
        mode = (MODE)sqlcfg->get_para()->menu_l2;
    }
    else{
        mode = Disable;
    }

    //如果是自动录波,需要改变模式
    if(this->_mode == Disable && mode != Disable){          //判自动录波
        //自动录波且非连续录波,且录波通道没被占用(这里困扰了很长时间,不加这个判断会出现波形不完整的bug,解决时间:2018-9-17)
        if(channel_h1->status == RecWave::Free && channel_h2->status == RecWave::Free
                && this->_mode != TEV_CONTINUOUS1 && this->_mode != TEV_CONTINUOUS2
                && this->_mode != HFCT_CONTINUOUS1 && this->_mode != HFCT_CONTINUOUS2
                && this->_mode != UHF_CONTINUOUS1 && this->_mode != UHF_CONTINUOUS2
                && this->_mode != Double_Channel){
            if(timer_interval == NULL){         //首次运行,初始化
                timer_interval = new QTimer;
                timer_interval->setSingleShot(true);
//                qDebug()<<"init timer_interval";
            }
            if(timer_interval->remainingTime() > 0){        //触发间隔
//                qDebug()<<"tringer the interval! remain timer:"<<timer_interval->remainingTime();
                data->set_send_para(sp_rec_start_h1, 2);
                data->set_send_para(sp_rec_start_h2, 2);
                fifocontrol->send_para();
                data->set_send_para(sp_rec_start_h1, 0);
                data->set_send_para(sp_rec_start_h2, 0);
                fifocontrol->send_para();
                return;
            }
            else{           //判断自动录波成功触发,重置间隔
                timer_interval->start(sqlcfg->get_para()->auto_rec_interval * 1000);    //开始设置间隔
                this->_mode = mode;
            }
        }
    }

    switch (mode) {
    case TEV1:
    case HFCT1:
    case UHF1:
        if(channel_h1->status == RecWave::Free){
            channel_h1->status = RecWave::Working;
            channel_h1->startWork();
        }
        else{
            channel_h1->work();
        }
        break;
    case TEV2:
    case HFCT2:
    case UHF2:
        if(channel_h2->status == RecWave::Free){
            channel_h2->status = RecWave::Working;
            channel_h2->startWork();
        }
        else{
            channel_h2->work();
        }
        break;
    case AA1:
    case AE1:
        if(channel_l1->status == RecWave::Free){
            channel_l1->status = RecWave::Working;
            channel_l1->startWork();
        }
        else{
            channel_l1->work();
        }
        break;
    case AA2:
    case AE2:
        if(channel_l2->status == RecWave::Free){
            channel_l2->status = RecWave::Working;
            channel_l2->startWork();
        }
        else{
            channel_l2->work();
        }
        break;
    default:
        break;
    }
}

MODE RecControl::mode()
{
    return _mode;
}

void RecControl::re_send_rec_continuous()
{
    rec_continuous_free_time = QTime::currentTime();
    hfct_rec_times = 0;

    switch (_mode) {
    case TEV_CONTINUOUS1:
        channel_h1->recStart(TEV1);
        break;
    case HFCT_CONTINUOUS1:
        channel_h1->recStart(HFCT1);
        break;
    case UHF_CONTINUOUS1:
        channel_h1->recStart(UHF1);
        break;
    case TEV_CONTINUOUS2:
        channel_h2->recStart(TEV2);
        break;
    case HFCT_CONTINUOUS2:
        channel_h2->recStart(HFCT2);
        break;
    case UHF_CONTINUOUS2:
        channel_h2->recStart(UHF2);
        break;
    default:
        break;
    }
//    if(_mode == HFCT_CONTINUOUS1){
//        channel_h1->recStart(HFCT1);
//    }
//    else if(_mode == HFCT_CONTINUOUS2){
//        channel_h2->recStart(HFCT2);
//    }
}

int RecControl::free_time()
{
//    return hfct_rec_times;
    return rec_continuous_free_time.msecsTo(QTime::currentTime() );
}

//录播完成的处理
//使用了一次性执行的多线程处理模式——QRunnable
//经实际测试，线程能自动结束
void RecControl::rec_wave_complete(VectorList wave, MODE mode)
{
//    qDebug()<<"rec_double_flag= "<<rec_double_flag;

    if(rec_double_flag == 2){                       //双通道波形第一部分
        rec_double = wave;
        rec_double_flag -- ;
    }
    else if(rec_double_flag == 1){                  //双通道波形第二部分
        rec_double.append(wave);
        rec_double_flag -- ;
        emit waveData(rec_double,Double_Channel);   //发送拼接好的双通道数据(保存操作在界面完成)
        this->_mode = Disable;
    }
    else if(timer_long->isActive()){                //连续录波途中,拼接
        re_send_rec_continuous();
        rec_continuous.append(wave);
    }
    else if(this->_mode == Disable){

    }
    else if(this->_mode != Disable){                //普通录波,连续录波完成,开启波形保存
        wave = Common::set_filter(wave, mode);      //高频录波加入滤波器
        mode_envelope_modify(mode);
        emit waveData(wave,mode);
        this->_mode = Disable;
        qDebug()<<"save wave file, mode = "<<Common::mode_to_string(mode);

        FileTools *filetools = new FileTools(wave,mode,FileTools::Write);      //开一个线程，为了不影响数据接口性能
        QThreadPool::globalInstance()->start(filetools);
    }
}

void RecControl::rec_continuous_complete()
{
    emit waveData(rec_continuous, _mode);
    qDebug()<<"rec continuous complete, times ============================================================== "<< rec_continuous.length()/4000;

    FileTools *filetools = new FileTools(rec_continuous,_mode,FileTools::Write);      //保存录波,单开一个临时线程，为了不影响数据接口性能
    _mode = Disable;         //Disable为去掉最后一次连续录波数据
    QThreadPool::globalInstance()->start(filetools);
}

void RecControl::mode_envelope_modify(MODE &mod)
{
    if(mod == AA1 && sqlcfg->get_para()->aa1_sql.envelope == true){
        mod = AA_ENVELOPE1;
    }
    else if(mod == AA2 && sqlcfg->get_para()->aa2_sql.envelope == true){
        mod = AA_ENVELOPE2;
    }
    else if(mod == AE1 && sqlcfg->get_para()->ae1_sql.envelope == true){
        mod = AE_ENVELOPE1;
    }
    else if(mod == AE2 && sqlcfg->get_para()->ae2_sql.envelope == true){
        mod = AE_ENVELOPE2;
    }

}





















