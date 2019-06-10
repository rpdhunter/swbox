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

//    uartmodbus = new UartModbus;

    reset_vol();
}

void LChannelFunction::channel_start()
{
    timer_1000ms->start();
    timer_100ms->start();
}

void LChannelFunction::change_vol(int d)
{
    Common::change_index(channel_sql->vol, d, VOL_MAX, VOL_MIN );
    reset_vol();
}

void LChannelFunction::reset_vol()
{
//    qDebug()<<"channel_num"<<channel_num;
    if(_channel_num == 3){
        data->set_send_para (sp_vol_l1, channel_sql->vol);
        data->set_send_para (sp_aa_record_play, 0);        //耳机送1通道
    }
    else if(_channel_num == 4){
        data->set_send_para (sp_vol_l2, channel_sql->vol);
        data->set_send_para (sp_aa_record_play, 2);        //耳机送2通道
    }
}

void LChannelFunction::save_rdb_data()
{
    switch (mode) {
    case AA1:
        Common::rdb_set_yc_value(AA1_amplitude_yc,show_val,_is_current);
        Common::rdb_set_yc_value(AA1_noise_biased_adv_yc,sug_noise_offset,_is_current);
        Common::rdb_set_yc_value(AA1_50Hz_yc,v_50Hz(),_is_current);
        Common::rdb_set_yc_value(AA1_100Hz_yc,v_100Hz(),_is_current);
        Common::rdb_set_yc_value(AA1_effective_yc,v_effective(),_is_current);
        Common::rdb_set_yc_value(AA1_peak_yc,v_peak(),_is_current);
        save_rdb_prpd(AA1_RPRD1_yc, _pulse_1000ms);
        break;
    case AA2:
        Common::rdb_set_yc_value(AA2_amplitude_yc,show_val,_is_current);
        Common::rdb_set_yc_value(AA2_noise_biased_adv_yc,sug_noise_offset,_is_current);
        Common::rdb_set_yc_value(AA2_50Hz_yc,v_50Hz(),_is_current);
        Common::rdb_set_yc_value(AA2_100Hz_yc,v_100Hz(),_is_current);
        Common::rdb_set_yc_value(AA2_effective_yc,v_effective(),_is_current);
        Common::rdb_set_yc_value(AA2_peak_yc,v_peak(),_is_current);
        save_rdb_prpd(AA2_RPRD1_yc, _pulse_1000ms);
        break;
    case AE1:
        Common::rdb_set_yc_value(AE1_amplitude_yc,show_val,_is_current);
        Common::rdb_set_yc_value(AE1_noise_biased_adv_yc,sug_noise_offset,_is_current);
        Common::rdb_set_yc_value(AE1_50Hz_yc,v_50Hz(),_is_current);
        Common::rdb_set_yc_value(AE1_100Hz_yc,v_100Hz(),_is_current);
        Common::rdb_set_yc_value(AE1_effective_yc,v_effective(),_is_current);
        Common::rdb_set_yc_value(AE1_peak_yc,v_peak(),_is_current);
        save_rdb_prpd(AE1_RPRD1_yc, _pulse_1000ms);
        break;
    case AE2:
        Common::rdb_set_yc_value(AE2_amplitude_yc,show_val,_is_current);
        Common::rdb_set_yc_value(AE2_noise_biased_adv_yc,sug_noise_offset,_is_current);
        Common::rdb_set_yc_value(AE2_50Hz_yc,v_50Hz(),_is_current);
        Common::rdb_set_yc_value(AE2_100Hz_yc,v_100Hz(),_is_current);
        Common::rdb_set_yc_value(AE2_effective_yc,v_effective(),_is_current);
        Common::rdb_set_yc_value(AE2_peak_yc,v_peak(),_is_current);
        save_rdb_prpd(AE2_RPRD1_yc, _pulse_1000ms);
        break;
    default:
        break;
    }

//    if((mode == AE1) || (mode == AE2)){
//        int i;
//        int len;
//        if(uartmodbus != NULL){
//            AE_send data;
//            data.mode = uartmodbus->Converse16(mode);
//            data.show_val = uartmodbus->Converse32((int)(show_val*10));
//            data.sug_noise_offset = uartmodbus->Converse32((int)(sug_noise_offset*10));
//            data.v_50Hz = uartmodbus->Converse32((int)(v_50Hz()*10));
//            data.v_100Hz = uartmodbus->Converse32((int)(v_100Hz()*10));
//            data.v_effective =  uartmodbus->Converse32((int)(v_effective()*10));
//            data.v_peak =  uartmodbus->Converse32((int)(v_peak()*10));

//            for (i = 0; i < _pulse_1000ms.count() && i < 40; ++i) {
//                data.prpdbuf[i] = Common::merge_int32(_pulse_1000ms.at(i).y(), _pulse_1000ms.at(i).x()%360);
//            }
//            len = sizeof(data) - sizeof(data.prpdbuf) + i*sizeof(int);
//            memcpy(buf,(char *)&data,len);
//            uartmodbus->modbus_AE_send(buf,len);
//        }
//    }

    _pulse_1000ms.clear();
}

void LChannelFunction::save_modbus_data()
{

}

void LChannelFunction::clear_100ms(){           //需要父函数调用
    _pulse_100ms.clear();
//    qDebug()<<"envelope_datalist"<<envelope_datalist.count()/128;
    envelope_datalist.clear();
    envelope_timelist.clear();
}

void LChannelFunction::clear_1000ms()
{
    _v_50Hz.clear();
    _v_100Hz.clear();
    _v_effective.clear();
    _v_peak.clear();
}

QVector<int> LChannelFunction::spectra_100ms(){       //频谱数据
    return envelope_datalist;
}

float LChannelFunction::v_50Hz()  const
{
    return qAbs(Compute::max(_v_50Hz) );
}

float LChannelFunction::v_100Hz()  const
{
    return qAbs(Compute::max(_v_100Hz) );
}

float LChannelFunction::v_effective() const
{
    return qAbs(Compute::max(_v_effective) );
}

float LChannelFunction::v_peak() const
{
    return qAbs(Compute::max(_v_peak) );
}

void LChannelFunction::compute_100ms()
{
    if(is_in_test_window()){
        _pulse_100ms = Compute::calc_pulse_list(envelope_datalist, envelope_timelist, channel_sql->fpga_threshold, mode, 10);    //计算脉冲
        compute_db_100ms();
        if(qAbs(db_val - temp_db ) >= channel_sql->step){       //阶梯显示

            emit fresh_gusty();
            temp_db = db_val;
            _max_val = MAX(_max_val, db_val);
        }

        _pulse_1000ms.append(_pulse_100ms);
    //    qDebug()<<"_pulse_1000ms:"<<_pulse_1000ms.count()<<Compute::max(_pulse_1000ms);

    //    qDebug()<<"envelope_datalist"<<envelope_datalist.count();
        float v50, v100, effective;
        if(envelope_datalist.count() > 1600){
            ae_fftlist = Compute::interpolation(envelope_datalist,1600,1024);       //差值得到1024点
            Compute::compute_f_value(ae_fftlist,fft,v50,v100,effective,mode);

        }
    //    qDebug()<<v50 <<"\t"<<v100;
        _v_50Hz.append(v50);
        _v_100Hz.append(v100);
        _v_effective.append(effective);
        _v_peak.append(Common::physical_value(Compute::max(envelope_datalist),mode));

        ae_fftlist.clear();
    }

    emit fresh_100ms();
}

void LChannelFunction::compute_1000ms()
{
    if(is_in_test_window()){
        show_val = last_1000ms_max_val;
        last_1000ms_max_val = -20;
        _max_val = MAX(_max_val, db_val);
    }

    emit fresh_1000ms();    
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
    if(is_in_test_window()){
        envelope_datalist.append(list.mid(5,128));
        envelope_timelist.append(list.at(2));
    }
}

//完成了计算实际值,db值和噪声建议值
void LChannelFunction::compute_db_100ms()
{
    int d;

    if(_channel_num == 3){
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
