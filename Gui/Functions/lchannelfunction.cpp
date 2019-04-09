#include "lchannelfunction.h"

LChannelFunction::LChannelFunction(G_PARA *data, MODE mode, QObject *parent)
    : BaseChannlFunction(data, mode, parent)
{
    timer_100ms = new QTimer(this);
    timer_100ms->setInterval(100);
    connect(timer_100ms, SIGNAL(timeout()), this, SLOT(compute_100ms()));

    timer_1000ms = new QTimer(this);
    timer_1000ms->setInterval(1000);
    connect(timer_1000ms, SIGNAL(timeout()), this, SLOT(compute_1000ms()));

    fft = new FFT;
    last_1000ms_max_val = -20;

    reset_vol();
}

void LChannelFunction::channel_start()
{
    BaseChannlFunction::channel_start();
}

void LChannelFunction::change_vol(int d)
{
    Common::change_index(channel_sql->vol, d, VOL_MAX, VOL_MIN );
    reset_vol();
}

void LChannelFunction::reset_vol()
{
    if(channel_num == 3){
        data->set_send_para (sp_vol_l1, channel_sql->vol);
        data->set_send_para (sp_aa_record_play, 0);        //耳机送1通道
    }
    else if(channel_num == 4){
        data->set_send_para (sp_vol_l2, channel_sql->vol);
        data->set_send_para (sp_aa_record_play, 2);        //耳机送2通道
    }
}

void LChannelFunction::save_rdb_data()
{
    switch (mode) {
    case AA1:
        Common::rdb_set_yc_value(AA1_amplitude_yc,show_val,_is_current);
//            Common::rdb_set_yc_value(AA1_severity,0,is_current);
//            Common::rdb_set_yc_value(AA1_gain,l_fun->sql()->gain,is_current);
//            Common::rdb_set_yc_value(AA1_biased,l_fun->sql()->offset,is_current);
        Common::rdb_set_yc_value(AA1_noise_biased_adv_yc,sug_noise_offset,_is_current);
        break;
    case AA2:
        Common::rdb_set_yc_value(AA2_amplitude_yc,show_val,_is_current);
        Common::rdb_set_yc_value(AA2_noise_biased_adv_yc,sug_noise_offset,_is_current);
        break;
    case AE1:
        Common::rdb_set_yc_value(AE1_amplitude_yc,show_val,_is_current);
        Common::rdb_set_yc_value(AE1_noise_biased_adv_yc,sug_noise_offset,_is_current);
        break;
    case AE2:
        Common::rdb_set_yc_value(AE2_amplitude_yc,show_val,_is_current);
        Common::rdb_set_yc_value(AE2_noise_biased_adv_yc,sug_noise_offset,_is_current);
        break;
    default:
        break;
    }
}

void LChannelFunction::clear_100ms(){           //需要父函数调用
    _pulse_100ms.clear();
//    qDebug()<<"envelope_datalist"<<envelope_datalist.count()/128;
    envelope_datalist.clear();
    envelope_timelist.clear();
}

QVector<int> LChannelFunction::spectra_100ms(){       //频谱数据
    return envelope_datalist;
}

int LChannelFunction::v_50Hz(){
    return _v_50Hz;
}

int LChannelFunction::v_100Hz(){
    return _v_100Hz;
}

void LChannelFunction::compute_100ms()
{
    _pulse_100ms = Common::calc_pulse_list(envelope_datalist, envelope_timelist, channel_sql->fpga_threshold, mode, 10);    //计算脉冲
    compute_db_100ms();
    if(qAbs(db_val - temp_db ) >= channel_sql->step){       //阶梯显示

        emit fresh_gusty();
        temp_db = db_val;
        _max_val = MAX(_max_val, db_val);
    }

//    qDebug()<<"envelope_datalist"<<envelope_datalist.count() / 128;
    for (int i = 0; i < envelope_datalist.count(); ++i) {
        if(i%25 == 0){
            ae_fftlist.append(envelope_datalist.at(i));
        }
    }

    emit fresh_100ms();
}

void LChannelFunction::compute_1000ms()
{
    show_val = last_1000ms_max_val;
//    qDebug()<<"last_1000ms_max_val"<<last_1000ms_max_val;




//    qDebug()<<"ae_fftlist"<<ae_fftlist.count();
    Compute::compute_f_value(ae_fftlist,fft,_v_50Hz,_v_100Hz, mode);
    ae_fftlist.clear();

//    qDebug()<<_v_50Hz <<"\t"<<_v_100Hz;
    emit fresh_1000ms();
    last_1000ms_max_val = -20;
    _max_val = MAX(_max_val, db_val);
}

/******************************************************************
 * 新的包络线数据读取接口
 * 1.一次接收128个点的包络线数据
 * 2.128点是3.2ms的数据(对应相位是57.6°)
 * 3.每秒接收312.5次,匀速
 * 一次传输固定为134个整形,格式如下:
 * 0--fpga缓冲区有无数据标志,1还有数据,2为读取完成
 * 1--组号,0-7循环变化
 * 2--时标（0-10M对应0-5×360°）
 * 3,4--废数据,等于上次接收数据的最后一位
 * 5-132--有效数据,共128位
 * 133--结束符0x55AA
 * ***************************************************************/
void LChannelFunction::read_envelope_data(VectorList list)
{
    envelope_datalist.append(list.mid(5,128));
    envelope_timelist.append(list.at(2));
}

//完成了计算实际值,db值和噪声建议值
void LChannelFunction::compute_db_100ms()
{
    int d;

    if(channel_num == 3){
        d = ((int)data->recv_para_normal.ldata0_max - (int)data->recv_para_normal.ldata0_min) / 2 ;      //(最大值-最小值)/2=幅值
    }
    else {
        d = ((int)data->recv_para_normal.ldata1_max - (int)data->recv_para_normal.ldata1_min) / 2 ;      //(最大值-最小值)/2=幅值
    }

    double factor = AA_FACTOR;
    if(mode == AE1){
        factor = sqlcfg->ae1_factor();
    }
    else if(mode == AE2){
        factor = sqlcfg->ae2_factor();
    }

    sug_noise_offset = ( d - 1 / channel_sql->gain / factor ) / 100;
    mv_val = Compute::l_channel_modify( (d - channel_sql->offset_noise * 100) * channel_sql->gain * factor );
    mv_val = MAX(mv_val, 0.1);      //保证结果有值，最小是-20dB
    db_val = 20 * log10 (mv_val);
    last_1000ms_max_val = MAX(last_1000ms_max_val, db_val);
    show_val = db_val;
}
