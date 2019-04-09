#include "basechannlfunction.h"

BaseChannlFunction::BaseChannlFunction(G_PARA *data, MODE mode, QObject *parent) : QObject(parent)
{
    this->data = data;
    this->mode = mode;
    this->_is_current = 0;
    this->db_val = 0;
    this->_max_val = 0;
    fir = new Fir;

    channel_init();
}

void BaseChannlFunction::channel_init()
{
    sql_para = *sqlcfg->get_para();         //内容复制,非指针复制,可以做到设置预览,设置和保存分离

    switch (mode) {
    case TEV1:
        channel_sql = &sql_para.tev1_sql;
        short_data = &data->recv_para_short1;
        channel_num = 1;
        break;
    case TEV2:
        channel_sql = &sql_para.tev2_sql;
        short_data = &data->recv_para_short2;
        channel_num = 2;
        break;
    case HFCT1:
        channel_sql = &sql_para.hfct1_sql;
        short_data = &data->recv_para_short1;
        channel_num = 1;
        break;
    case HFCT2:
        channel_sql = &sql_para.hfct2_sql;
        short_data = &data->recv_para_short2;
        channel_num = 2;
        break;
    case UHF1:
        channel_sql = &sql_para.uhf1_sql;
        short_data = &data->recv_para_short1;
        channel_num = 1;
        break;
    case UHF2:
        channel_sql = &sql_para.uhf2_sql;
        short_data = &data->recv_para_short2;
        channel_num = 2;
        break;
    case AA1:
        channel_sql = &sql_para.aa1_sql;
        envelope_data = &data->recv_para_envelope1;
        channel_num = 3;
        break;
    case AA2:
        channel_sql = &sql_para.aa2_sql;
        envelope_data = &data->recv_para_envelope2;
        channel_num = 4;
        break;
    case AE1:
        channel_sql = &sql_para.ae1_sql;
        envelope_data = &data->recv_para_envelope1;
        channel_num = 3;
        break;
    case AE2:
        channel_sql = &sql_para.ae2_sql;
        envelope_data = &data->recv_para_envelope2;
        channel_num = 4;
        break;
    default:
        break;
    }
}

void BaseChannlFunction::channel_start()
{
    if(channel_sql->mode == continuous){
        timer_100ms->setSingleShot(false);
        timer_1000ms->setSingleShot(false);
    }
    else{
        timer_100ms->setSingleShot(true);
        timer_1000ms->setSingleShot(true);
    }

    if(!timer_1000ms->isActive()){
        timer_1000ms->start();
    }
    if(!timer_100ms->isActive()){
        timer_100ms->start();
    }
}

void BaseChannlFunction::toggle_test_status()
{
    sqlcfg->get_global()->test_mode = !sqlcfg->get_global()->test_mode;
}

void BaseChannlFunction::change_chart(int d)
{
    QList<int> list;
    switch (mode) {
    case TEV1:
    case TEV2:
        list << BASIC << PRPD << PRPS << Histogram;
        break;
    case HFCT1:
    case HFCT2:
        list << BASIC << PRPD << PRPS << TF << Spectra;
        break;
    case UHF1:
    case UHF2:
        list << BASIC << PRPD << PRPS << Histogram;
        break;
    case AA1:
    case AA2:
        if(channel_sql->camera){
            list << BASIC << PRPD << PRPS << Spectra << Camera;
        }
        else{
            list << BASIC << PRPD << PRPS << Spectra;
        }
        break;
    case AE1:
    case AE2:
        list << BASIC << PRPD  << PRPS << FLY << Exponent << Spectra;
        break;
    default:
        break;
    }
    Common::change_index(channel_sql->chart,d,list);
}

void BaseChannlFunction::change_threshold(int d)
{
//    Common::change_index(channel_sql->fpga_threshold, d * Common::code_value(1,mode), Common::code_value(100,mode), Common::code_value(2,mode) );
    Common::change_index(channel_sql->fpga_threshold, d, 100, 2 );
}

void BaseChannlFunction::change_rec_cons_time(int d)
{
    Common::change_index(channel_sql->rec_time, d, 5, 1 );
}

void BaseChannlFunction::toggle_units()
{

}

void BaseChannlFunction::click_reset()
{
    _max_val = 0;
}

void BaseChannlFunction::toggle_test_mode()
{
    channel_sql->mode = !channel_sql->mode;
    if (channel_sql->mode == single) {
        timer_1000ms->setSingleShot(true);
        timer_100ms->setSingleShot(true);
    } else {
        timer_1000ms->setSingleShot(false);
        timer_100ms->setSingleShot(false);
    }
}

void BaseChannlFunction::change_gain(int d)
{
//    Common::change_index(channel_sql->gain, d * 0.1, 20, 0.1 );
    if( (channel_sql->gain<9.95 && d>0) || (channel_sql->gain<10.15 && d<0) ){
        Common::change_index(channel_sql->gain, d * 0.1, 100, 0.1 );
    }
    else{
        Common::change_index(channel_sql->gain, d * 10, 100, 0.1 );
    }
}

void BaseChannlFunction::change_red_alarm(int d)
{
    if(channel_sql->units == Units_pC){
        Common::change_index(channel_sql->high, d*100, 9000, channel_sql->low );
    }
    else{
        Common::change_index(channel_sql->high, d, 60, channel_sql->low );
    }
}

void BaseChannlFunction::change_yellow_alarm(int d)
{
    if(channel_sql->units == Units_pC){
        Common::change_index(channel_sql->low, d*100, channel_sql->high, 0 );
    }
    else{
        Common::change_index(channel_sql->low, d, channel_sql->high, 0 );
    }
}

void BaseChannlFunction::toggle_buzzer()
{

}

void BaseChannlFunction::change_pulse_time(int d)
{
    Common::change_index(channel_sql->pulse_time, d, MAX_PULSE_CNT, 1 );
}

void BaseChannlFunction::toggle_mode_recognition()
{

}

void BaseChannlFunction::set_current(int is_current)
{
    _is_current = is_current;
    switch (channel_num) {
    case 3:
        data->set_send_para (sp_aa_record_play, 0);        //耳机送1通道
        break;
    case 4:
        data->set_send_para (sp_aa_record_play, 2);        //耳机送2通道
        break;
    default:
        break;
    }
}

void BaseChannlFunction::save_sql()
{
    sqlcfg->sql_save(&sql_para);
}

void BaseChannlFunction::cancel_sql()
{
    sql_para = *sqlcfg->get_para();
}

int BaseChannlFunction::val(){
    return show_val;
}

int BaseChannlFunction::max_val(){
    return _max_val;
}

int BaseChannlFunction::pulse_cnt(){
    return _pulse_cnt_show;
}

float BaseChannlFunction::degree(){
    return _degree;
}

CHANNEL_SQL *BaseChannlFunction::sql(){
    return channel_sql;
}

QVector<QPoint> BaseChannlFunction::pulse_100ms(){
    return _pulse_100ms;
}
