#include "hchannelfunction.h"

#include "Algorithm/compute.h"
#include "Algorithm/Wavelet/wavelet.h"

HChannelFunction::HChannelFunction(G_PARA *data, MODE mode, QObject *parent)
    : BaseChannlFunction(data, mode, parent)
{
    this->token = 0;
    this->max_100ms = 0;

    timer_token = new QTimer(this);         //令牌计时器
    timer_token->setInterval(2);
    connect(timer_token, SIGNAL(timeout()), this, SLOT(add_token()));

    timer_100ms = new QTimer(this);
    timer_100ms->setInterval(100);    
    connect(timer_100ms, SIGNAL(timeout()), this, SLOT(compute_100ms()));

    timer_1000ms = new QTimer(this);
    timer_1000ms->setInterval(1000);
    connect(timer_1000ms, SIGNAL(timeout()), this, SLOT(compute_1000ms()));


    reset_auto_rec();               //自动录波
    reset_threshold();              //设置阈值
}

void HChannelFunction::channel_start()
{
    BaseChannlFunction::channel_start();
    if(!timer_token->isActive()){
        timer_token->start();
    }    
}

void HChannelFunction::change_threshold(int d)
{
    BaseChannlFunction::change_threshold(d);
    reset_threshold();
}

void HChannelFunction::reset_threshold()
{
    if( channel_num == 1){
        data->set_send_para(sp_h1_threshold, Common::code_value(channel_sql->fpga_threshold, mode));
    }
    else if(channel_num == 2){
        data->set_send_para(sp_h2_threshold, Common::code_value(channel_sql->fpga_threshold, mode));
    }
}

void HChannelFunction::toggle_auto_rec()
{
    channel_sql->auto_rec = !channel_sql->auto_rec;
    reset_auto_rec();
}

void HChannelFunction::reset_auto_rec()
{
    if(channel_sql->auto_rec == true){
        data->set_send_para (sp_rec_on, 1);
        data->set_send_para(sp_auto_rec, channel_num);
    }
    else{
        data->set_send_para(sp_auto_rec, 0);
    }
}

void HChannelFunction::change_HF(int d)
{
    QList<int> list;
    list << NONE << hp_500k << hp_1M << hp_1M5 << hp_1M8 << hp_2M << hp_2M5 << hp_3M << hp_5M
         << hp_8M << hp_10M << hp_12M << hp_15M << hp_18M << hp_20M << hp_22M << hp_25M << hp_28M
         << hp_30M << hp_32M << hp_35M;
    Common::adjust_filter_list(list, 0, Common::filter_to_number(channel_sql->filter_lp));     //调整可选频率列表,使得高通截止频率永远低于低通
    Common::change_index(channel_sql->filter_hp, d, list);
}

void HChannelFunction::change_LF(int d)
{
    QList<int> list;
    list << NONE << lp_2M << lp_5M << lp_8M << lp_10M << lp_12M << lp_15M << lp_18M << lp_20M
         << lp_22M << lp_25M << lp_28M << lp_30M << lp_32M << lp_35M << lp_38M << lp_40M;
    Common::adjust_filter_list(list, Common::filter_to_number(channel_sql->filter_hp), 100);
    Common::change_index(channel_sql->filter_lp, d, list);
}

void HChannelFunction::change_gain(int d)
{
    BaseChannlFunction::change_gain(d);
    reset_threshold();              //注意前端增益修改之后,应当调整对应FPGA的触发脉冲
}

void HChannelFunction::save_rdb_data()
{
    switch (mode) {
    case TEV1:
        Common::rdb_set_yc_value(TEV1_amplitude_yc,db_val,_is_current);
        Common::rdb_set_yc_value(TEV1_num_yc,_pulse_cnt_show,_is_current);
        Common::rdb_set_yc_value(TEV1_severity_yc,_degree,_is_current);
        Common::rdb_set_yc_value(TEV1_center_biased_adv_yc,sug_zero_offset,_is_current);
        Common::rdb_set_yc_value(TEV1_noise_biased_adv_yc,sug_noise_offset,_is_current);
        break;
    case TEV2:
        Common::rdb_set_yc_value(TEV2_amplitude_yc,db_val,_is_current);
        Common::rdb_set_yc_value(TEV2_num_yc,_pulse_cnt_show,_is_current);
        Common::rdb_set_yc_value(TEV2_severity_yc,_degree,_is_current);
        Common::rdb_set_yc_value(TEV2_center_biased_adv_yc,sug_zero_offset,_is_current);
        Common::rdb_set_yc_value(TEV2_noise_biased_adv_yc,sug_noise_offset,_is_current);
        break;
    case HFCT1:
        Common::rdb_set_yc_value(HFCT1_amplitude_yc,db_val,_is_current);
        Common::rdb_set_yc_value(HFCT1_num_yc,_pulse_cnt_show,_is_current);
        Common::rdb_set_yc_value(HFCT1_severity_yc,_degree,_is_current);
        Common::rdb_set_yc_value(HFCT1_center_biased_adv_yc,sug_zero_offset, _is_current);;
        Common::rdb_set_yc_value(HFCT1_noise_biased_adv_yc,0,_is_current);

        break;
    case HFCT2:
        Common::rdb_set_yc_value(HFCT2_amplitude_yc,db_val,_is_current);
        Common::rdb_set_yc_value(HFCT2_num_yc,_pulse_cnt_show,_is_current);
        Common::rdb_set_yc_value(HFCT2_severity_yc,_degree,_is_current);
        Common::rdb_set_yc_value(HFCT2_center_biased_adv_yc,sug_zero_offset, _is_current);
        Common::rdb_set_yc_value(HFCT2_noise_biased_adv_yc,0,_is_current);

        break;
    case UHF1:
        Common::rdb_set_yc_value(UHF1_amplitude_yc,db_val,_is_current);
        Common::rdb_set_yc_value(UHF1_num_yc,_pulse_cnt_show,_is_current);
        Common::rdb_set_yc_value(UHF1_severity_yc,_degree,_is_current);
        Common::rdb_set_yc_value(UHF1_center_biased_adv_yc,sug_zero_offset,_is_current);
        Common::rdb_set_yc_value(UHF1_noise_biased_adv_yc,sug_noise_offset,_is_current);
        break;
    case UHF2:
        Common::rdb_set_yc_value(UHF2_amplitude_yc,db_val,_is_current);
        Common::rdb_set_yc_value(UHF2_num_yc,_pulse_cnt_show,_is_current);
        Common::rdb_set_yc_value(UHF2_severity_yc,_degree,_is_current);
        Common::rdb_set_yc_value(UHF2_center_biased_adv_yc,sug_zero_offset,_is_current);
        Common::rdb_set_yc_value(UHF2_noise_biased_adv_yc,sug_noise_offset,_is_current);
        break;
    default:
        break;
    }
}

QVector<PC_DATA> HChannelFunction::pclist_100ms(){
    return _pclist_100ms;
}

void HChannelFunction::clear_100ms(){           //需要父函数调用
    _pulse_100ms.clear();
    _pclist_100ms.clear();
    _spectra_100ms.clear();
    max_100ms = 0;
}

void HChannelFunction::clear_1000ms(){
    amp_1000ms.clear();
}

QVector<int> HChannelFunction::spectra_100ms(){
    return _spectra_100ms;
}

/************************************************
 * 短录波读取类,完成基本的脉冲/放电量计算功能
 * 由fifo线程驱动,发生频率由FPGA检测到脉冲的频率决定
 * 由于发生频率的不可预估性,使用了令牌限速器,限制流入速度
 * **********************************************/
void HChannelFunction::read_short_data()
{
    if( short_time != short_data->time  && short_data->data_flag == 0 && short_data->time != 0x55aa){         //判断数据有效性
        short_time = short_data->time;

//        if(this->channel_num == 2)
//            qDebug()<<"token:" << token;

        //令牌检测
        if(token == 0){
            return;
        }
        else{
            token--;
        }

        //拷贝数据
        QVector<qint32> list;
        for (int i = 3; i < 256; ++i) {
            if(short_data->data[i] == 0x55aa){          //每次上送240-3个点(前三个点是无效数据)
                break;
            }
            else{
                list.append(((qint32)short_data->data[i] - 0x8000 - channel_sql->fpga_zero));
            }
        }

        //加入滤波器(目前仅hfct通道实装)
        list = Common::set_filter(list, mode);

        //计算db值,一次短录波只计算一个db值(可能需要修改,增加点数)
        QPoint max_P = Compute::find_max_point(list);                   //求出绝对值最大点
        int phase = Common::time_to_phase(short_data->time);            //相位
//        qDebug()<<max_P << QPoint(phase, code_to_db(max_P.y()) );
//        qDebug()<<Common::mode_to_string(mode)<<"list"<<list<<list.count();


//            qDebug()<<"phase"<<phase << "\t" << short_data->time;


        if( qAbs(max_P.y() ) > max_100ms ){
            _spectra_100ms = list;            //找到100ms中最大的一组脉冲数据进行频谱分析
            max_100ms = qAbs(max_P.y() );
        }



        //计算pC值,一次短录波只计算一个pC值
        if(channel_sql->units == Units_pC){
            QVector<int> max_list = Compute::find_max_peak(list, max_P);        //找出绝对值最大点附近完整波形
            PC_DATA pc_data = Compute::compute_pc_1node(max_list,phase,channel_sql->gain);    //计算这个波形的pC信息
            _pclist_100ms.append(pc_data);
            _pulse_100ms.append(QPoint(phase, pc_data.pc_value ));
        }
        else{
            _pulse_100ms.append(QPoint(phase, code_to_db(max_P.y()) )); //转为db值
//            if(_pulse_100ms.last().y() > 60){
//                qDebug()<<"_pulse_100ms:"<<QPoint(phase, (int)code_to_db(max_P.y()) )<<"\t"<<max_P;
//            }
        }
    }
}

void HChannelFunction::compute_100ms()
{
    foreach (QPoint p, _pulse_100ms) {
        amp_1000ms.append(qAbs(p.y()) );
    }

//    qDebug()<<Common::mode_to_string(mode)
//           <<"\tdb:"<< code_to_db(Common::max(_spectra_100ms)) <<"\t show_val:"<<show_val;

    emit fresh_100ms();
}

void HChannelFunction::compute_1000ms()
{    
    compute_pulse_1000ms();     //脉冲计数

    switch (channel_sql->units) {
    case Units_pC:
        pc_val = Common::max(amp_1000ms);       //找出最大值
        pc_val = MIN(pc_val, 9999);
//        qDebug()<<"show_val :"<< show_val<< amp_1000ms;
        show_val = pc_val;
        break;
    case Units_mV:
        compute_db_1000ms();
        mv_val = MIN(mv_val, 1000);
        show_val = mv_val;
        break;
    case Units_db:
        compute_db_1000ms();        //地电波强度
        db_val = MIN(db_val, 60);
        show_val = db_val;
        break;
    default:
        break;
    }

    amp_1000ms.clear();

    show_val = MAX(show_val, 0);
    _max_val = MAX(_max_val, show_val);        //最大值

    //严重度
    _degree = pow(show_val,channel_sql->gain) * (double)pulse_cnt_list.last() / sql_para.freq_val;     //严重度算法更改,严重度 = 幅值×每周期脉冲数

//       qDebug()<<Common::mode_to_string(mode)<<"_spectra_100ms" <<_spectra_100ms << _spectra_100ms.count()
//              <<"\ndb:"<< code_to_db(Common::max(_spectra_100ms)) <<"\t show_val:"<<show_val;
    emit fresh_1000ms();

}

void HChannelFunction::add_token()
{
    if(token < TOKEN_MAX){
        token += 1;
    }
}

void HChannelFunction::compute_db_1000ms()
{
    int d_max, d_min, a, b;
    if (channel_num == 1) {
        d_max = data->recv_para_normal.hdata0.ad.ad_max;
        d_min = data->recv_para_normal.hdata0.ad.ad_min;
    }
    else{
        d_max = data->recv_para_normal.hdata1.ad.ad_max;
        d_min = data->recv_para_normal.hdata1.ad.ad_min;
    }

//    qDebug()<<Common::mode_to_string(mode)<<"d_max="<<d_max - 0x8000<<"\td_min="<<d_min - 0x8000 << "\ttev_sql->fpga_zero="<<channel_sql->fpga_zero;

    sug_zero_offset = ((d_max + d_min) / 2) - 0x8000;

    a = d_max - 0x8000 - channel_sql->fpga_zero;        //减去中心偏置
    b = d_min - 0x8000 - channel_sql->fpga_zero;        //减去中心偏置

    sug_noise_offset = ( MAX (qAbs (a), qAbs (b)) - 1 / H_C_FACTOR / channel_sql->gain ) /10;

    mv_val = channel_sql->gain * (MAX (qAbs (a), qAbs (b)) - channel_sql->offset_noise * 10) * H_C_FACTOR;
    db_val = 20 * log10 (mv_val);      //对数运算，来自工具链的函数

    //脉冲数多时，进入测试模式^^
    if(pulse_cnt_list.last() > 500000 && !amp_1000ms.isEmpty()){
        db_val = Common::avrage(amp_1000ms);
        double k = Common::tev_freq_compensation(pulse_cnt_list.last());        //TEV补偿
        //        qDebug()<<"k="<<k;
        db_val += 20 * log10 (k);
    }
}

void HChannelFunction::compute_pulse_1000ms()
{
    quint32 pulse_cnt;
    if(channel_num == 1){
        pulse_cnt = data->recv_para_normal.hpulse0_totol;
    }
    else{
        pulse_cnt = data->recv_para_normal.hpulse1_totol;
    }
    pulse_cnt_list.append(pulse_cnt);
    if(pulse_cnt_list.count() > MAX_PULSE_CNT){
        pulse_cnt_list.removeFirst();
    }
    _pulse_cnt_show = 0;
    for (int i = 0; i < channel_sql->pulse_time && pulse_cnt_list.count() >= i+1; ++i) {
        _pulse_cnt_show += pulse_cnt_list.at(pulse_cnt_list.count() - 1 - i);
    }
}

double HChannelFunction::code_to_db(qint32 code_val)
{
    double y;
    y = channel_sql->gain * H_C_FACTOR * code_val;
    //取DB值
    if(y>1){
        y = 20.0 * log10(y);
    }
    else if(y<-1){
        y = -20.0 * log10(-y);
    }
    else{
        y = 0;
    }

//    if(qAbs(code_val) > 1000){
//        qDebug()<<"code_to_db"<<channel_sql->gain * H_C_FACTOR * code_val << "\t"<< y;
//    }
    return y;
}
