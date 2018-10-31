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
    switch (mode) {
    case TEV1:
    case HFCT1:
    case UHF1:
        channel_h1->recStart(mode);
        break;
    case TEV2:
    case HFCT2:
    case UHF2:
        channel_h2->recStart(mode);
        break;
    case HFCT1_CONTINUOUS:
        channel_h1->recStart(HFCT1);
        timer_long->start(time * 1000);
        rec_continuous.clear();
        break;
    case HFCT2_CONTINUOUS:
        channel_h2->recStart(HFCT2);
        timer_long->start(time * 1000);
        rec_continuous.clear();
        break;
    case AA1:
    case AE1:
        channel_l1->recStart(mode,time);
        break;
    case AA2:
    case AE2:
        channel_l2->recStart(mode,time);
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
        qDebug()<<"rece rec_double_signal";
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
        if(this->_mode != HFCT1_CONTINUOUS && this->_mode != HFCT2_CONTINUOUS && this->_mode != Double_Channel
                && channel_h1->status == RecWave::Free && channel_h2->status == RecWave::Free ){
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
    if(_mode == HFCT1_CONTINUOUS){
        channel_h1->recStart(HFCT1);
    }
    else if(_mode == HFCT2_CONTINUOUS){
        channel_h2->recStart(HFCT2);
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
void RecControl::rec_wave_complete(VectorList wave, MODE mode)
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
    }
    else if(timer_long->isActive()){
        re_send_rec_continuous();
        rec_continuous.append(wave);
    }
    else if(this->_mode == Disable){
//        this->_mode = mode;
    }
    else if(this->_mode != Disable){            //这里开启波形保存
        //高频录波加入滤波器
        Fir fir;
        if(mode == HFCT1 || mode == HFCT1_CONTINUOUS){
            wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->hfct1_sql.filter_hp);
            wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->hfct1_sql.filter_lp);
//            wave = Wavelet::set_filter(wave,1);     //小波滤波
        }
        else if(mode == HFCT2 || mode == HFCT2_CONTINUOUS){
            wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->hfct2_sql.filter_hp);
            wave = fir.set_filter(wave, (FILTER)sqlcfg->get_para()->hfct2_sql.filter_lp);
//            wave = Wavelet::set_filter(wave,1);     //小波滤波
        }


        emit waveData(wave,mode);
        this->_mode = Disable;
        qDebug()<<"save wave file, mode = "<<Common::MODE_toString(mode);

        FileTools *filetools = new FileTools(wave,mode,FileTools::Write);      //开一个线程，为了不影响数据接口性能
        QThreadPool::globalInstance()->start(filetools);
    }
}

void RecControl::rec_continuous_complete()
{
    emit waveData(rec_continuous, _mode);
    qDebug()<<"rec continuous complete, times ============================================================== "<< rec_continuous.length()/4000;

    //这里插入FFT测试程序
//    fft_test(rec_continuous);

    FileTools *filetools = new FileTools(rec_continuous,_mode,FileTools::Write);      //开一个线程，为了不影响数据接口性能
    _mode = Disable;         //Disable为去掉最后一次连续录波数据
    QThreadPool::globalInstance()->start(filetools);
}

void RecControl::fft_test(VectorList inputlist)
{
    VectorList tmplist = inputlist.mid(250,32);


    for (int i = 0; i < 4000 / 32 - 1 ; ++i) {
        for (int j = 0; j < 32; ++j) {
            inputlist[i*32 + j] = tmplist.at(j);
        }
    }

    qDebug()<<inputlist;


    FFT fft;
    int fft_in[2048];
    VectorList list;

    QTime t;            //计时器
    t.start();

    for (int i = 0; i < inputlist.count()/4000; ++i) {
        for (int j = 0; j < 2048; ++j) {
            fft_in[j] = inputlist.at(i*4000 + j);
        }
        list.append( fft.fft2048(fft_in) );

    }

    qDebug("Time elapsed: %d ms", t.elapsed());

    qDebug()<<"fft complete! length = "<<list.length();
    qDebug()<<list;


}





















