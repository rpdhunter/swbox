#include "channelwidget.h"

ChannelWidget::ChannelWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) : BaseWidget(val,parent)
{
    this->h_fun = NULL;
    this->l_fun = NULL;
    this->fun = NULL;
    this->data = data;
    this->mode = mode;
    this->menu_index = menu_index;
    this->manual = false;

    timer_freeze = new QTimer(this);      //timer3设置了一个界面手动退出后的锁定期,便于操作
    timer_freeze->setInterval(FREEZE_TIME);      //1.2秒内不出现新录波界面
    timer_freeze->setSingleShot(true);

    recWaveForm = new RecWaveForm(menu_index,this);
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));
    connect(recWaveForm,SIGNAL(fresh_parent()),this,SLOT(doHideWaveData()));
//    connect(recWaveForm,SIGNAL(fresh_parent()),this,SIGNAL(fresh_parent()));
//    connect(recWaveForm, SIGNAL(fresh_parent()), timer_freeze, SLOT(start()) );

    logtools = new LogTools(mode);      //日志保存模块
    connect(this,SIGNAL(send_log_data(double,int,double,int,QString)),logtools,SLOT(dealLog(double,int,double,int,QString)));
    connect(this,SIGNAL(send_PRPD_data(QVector<QwtPoint3D>)),logtools,SLOT(dealRPRDLog(QVector<QwtPoint3D>)));

    historic_chart = new HistoricChart(this);
    prpd_chart = new PRPDChart(this);
    prps_chart = new PRPSChart(this);
    spectra_chart = new SpectraChart(this);
    histogram_chart = new HistogramChart(this);    
    tf_chart = new TFChart(this);
    fly_chart = new FlyChart(this);
    camera_chart = new CameraChart(this);

}

void ChannelWidget::change_log_dir()
{
    logtools->change_current_asset_dir();
}

/****************************************************
 * 录波界面显示规则
 * 1.只显示当前通道的录波结果,非当前通道不显示
 * 2.手动录波必须显示
 * 3.自动录波显示优先级低于设置菜单(使得更改设置可以从容不迫)
 *   摒弃了之前冻结5秒的傻逼设计
 * *************************************************/
void ChannelWidget::showWaveData(VectorList buf, MODE mod)
{

    if(key_val->grade.val0 == menu_index && menu_index == Common::mode_to_channel(mod) ){     //当前通道

//        qDebug()<<"get rec WaveData"<<Common::mode_to_string(mod)
//               << "key_val->grade.val0" << key_val->grade.val0 << "\tmenu_index"<<menu_index;
        if(manual == true){         //手动录波
            manual = false;
            emit show_indicator(false);
            key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
            key_val->grade.val5 = 1;
            emit fresh_parent();
            recWaveForm->working(key_val,buf,mod);
        }
        else{                       //自动录波
            if(key_val->grade.val1 == 0 || !recWaveForm->isHidden()){
                if(timer_freeze->isActive()){
                    return;
                }
                emit show_indicator(false);
                key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
                key_val->grade.val5 = 1;
                emit fresh_parent();
                recWaveForm->working(key_val,buf,mod);
            }
        }
    }
}

void ChannelWidget::save_channel()
{
    logtools->save_log();
    prpd_chart->save_data();
}

void ChannelWidget::set_current(int index)
{
    int is_current = 0;
    if(index == menu_index){
        is_current = 1;
    }

    if(fun != NULL){
        fun->set_current(is_current);
        fun->cancel_sql();
    }
}

void ChannelWidget::fresh_1000ms()
{
    qDebug()<<"ChannelWidget::fresh_1000ms";
}

void ChannelWidget::fresh_100ms()
{
    qDebug()<<"ChannelWidget::fresh_100ms";
}

void ChannelWidget::trans_key(quint8 key_code)
{
    if(key_val->grade.val0 != menu_index) {
        return;
    }

    if(key_val->grade.val5 != 0){
        emit send_key(key_code);
        return;
    }

    BaseWidget::trans_key(key_code);
    fresh_setting();
}

void ChannelWidget::doHideWaveData()
{
    timer_freeze->start();
    fresh_setting();

}

void ChannelWidget::read_short_data()
{
    if(h_fun != NULL){
        h_fun->read_short_data();
    }
}

void ChannelWidget::read_envelope_data(VectorList list)
{
    if(l_fun != NULL){
        l_fun->read_envelope_data(list);
    }
}

void ChannelWidget::do_key_ok()
{
    key_val->grade.val1 = 0;
    key_val->grade.val2 = 0;
}

void ChannelWidget::do_key_cancel()
{
    key_val->grade.val1 = 0;
    key_val->grade.val2 = 0;

    if(fun != NULL){
        fun->cancel_sql();
        historic_chart->reset_colormap(fun->sql()->high, fun->sql()->low);
        prps_chart->reset_colormap(fun->sql()->high,fun->sql()->low);
    }

    if(h_fun != NULL){        
        h_fun->reset_auto_rec();
        h_fun->reset_threshold();
    }
}

void ChannelWidget::do_key_left_right(int d)
{
    if(fun != NULL){
        switch (key_val->grade.val2) {
        case SUB_MENU_NUM_BASE + 0:         //修改检测模式
            fun->toggle_test_mode();
            break;
        case SUB_MENU_NUM_BASE + 1:         //修改增益
            fun->change_gain(d);
            break;
        case SUB_MENU_NUM_BASE + 2:         //修改脉冲触发阈值
            fun->change_threshold(d);
            break;
        case SUB_MENU_NUM_BASE + 3:         //修改黄色报警阈值
            fun->change_yellow_alarm(d);
            historic_chart->reset_colormap(fun->sql()->high, fun->sql()->low);     //修改需要预先执行
            prps_chart->reset_colormap(fun->sql()->high,fun->sql()->low);
            break;
        case SUB_MENU_NUM_BASE + 4:         //修改红色报警阈值
            fun->change_red_alarm(d);
            historic_chart->reset_colormap(fun->sql()->high, fun->sql()->low);
            prps_chart->reset_colormap(fun->sql()->high,fun->sql()->low);
            break;
        case SUB_MENU_NUM_BASE + 5:         //修改脉冲计数时长
            fun->change_pulse_time(d);
            break;
        case SUB_MENU_NUM_BASE + 6:         //修改脉冲计数时长
            fun->toggle_mode_recognition();
            break;
        default:
            break;
        }
    }
}

void ChannelWidget::fresh_setting()
{
    historic_chart->hide();
    prpd_chart->hide();
    prps_chart->hide();
    spectra_chart->hide();
    histogram_chart->hide();
    tf_chart->hide();
    fly_chart->hide();
    camera_chart->hide();

    int chart = 0;
    if(fun != NULL){
        chart = fun->sql()->chart;
        settingMenu->fresh(fun->sql(),key_val->grade.val2);     //刷新菜单
    }

    switch (chart) {
    case BASIC:
        historic_chart->show();
        break;
    case PRPD:
        prpd_chart->show();
        break;
    case PRPS:
        prps_chart->show();
        break;
    case Histogram:
    case Exponent:
        histogram_chart->show();
        break;
    case TF:
        tf_chart->show();
        break;
    case FLY:
        fly_chart->show();
        break;
    case Spectra:
        spectra_chart->show();
        break;
    case Camera:        //待修改
        camera_chart->show();
        break;
    default:
        break;
    }

    emit fresh_parent();
}

void ChannelWidget::data_reset()
{

}




