#include "aawidget.h"
#include "ui_aawidget.h"
#include <QLineEdit>

#define SETTING_NUM 8           //设置菜单条目数
#define VALUE_MAX 60

AAWidget::AAWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    ChannelWidget(data, val, mode, menu_index, parent),
    ui(new Ui::AAWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("AAWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    Common::set_comboBox_style(ui->comboBox);

    recWaveForm->raise();
    reload(-1);
    chart_ini();
    reload(menu_index);
}

AAWidget::~AAWidget()
{
    delete ui;
}

void AAWidget::reload(int index)
{
    //基本sql内容的初始化
    sql_para = *sqlcfg->get_para();
    if(mode == AA1){
        aaultra_sql = &sql_para.aa1_sql;
        ae_pulse_data = &data->recv_para_envelope1;
    }
    else if(mode == AA2){
        aaultra_sql = &sql_para.aa2_sql;
        ae_pulse_data = &data->recv_para_envelope1;
    }

    //构造函数中计时器不启动
    if(index == menu_index){
        if(!timer_100ms->isActive()){
            timer_100ms->start();
        }
        if(!timer_1000ms->isActive()){
            timer_1000ms->start();
        }

        if(mode == AA1){
            data->set_send_para (sp_vol_l1, aaultra_sql->vol);
            data->set_send_para (sp_aa_record_play, 0);        //耳机送1通道
        }
        else if(mode == AA2){
            data->set_send_para (sp_vol_l2, aaultra_sql->vol);
            data->set_send_para (sp_aa_record_play, 2);        //耳机送2通道
        }
        data->set_send_para(sp_auto_rec, 0);        //关闭自动录波
        ae_pulse_data->readComplete = 1;        //读取完成标志
        fresh_setting();
    }
}

void AAWidget::chart_ini()
{
    //图形设置
    Common::set_barchart_style(ui->qwtPlot, VALUE_MAX);
    d_BarChart = new BarChart(ui->qwtPlot, &db, &aaultra_sql->high, &aaultra_sql->low);
    d_BarChart->resize(200, 140);

    //Spectra
    plot_Spectra = new QwtPlot(ui->widget);
    plot_Spectra->resize(200, 140);
    d_Spectra = new QwtPlotHistogram;
    Common::set_Spectra_style(plot_Spectra,d_Spectra,0,4,0,30,"");
}

void AAWidget::do_key_ok()
{
    sqlcfg->sql_save(&sql_para);
    reload(menu_index);
    switch (key_val->grade.val2) {
    case 7:
        key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
        emit startRecWave(mode, aaultra_sql->time);        //发送录波信号
        emit show_indicator(true);
//        isBusy = true;
        return;
    case 8:
        maxReset(ui->label_max);
        break;
    default:
        break;
    }
    ChannelWidget::do_key_ok();
}

void AAWidget::do_key_up_down(int d)
{
    key_val->grade.val1 = 1;
    Common::change_index(key_val->grade.val2, d, SETTING_NUM, 1);
}

void AAWidget::do_key_left_right(int d)
{
    QList<int> list;

    switch (key_val->grade.val2) {
    case 1:
        aaultra_sql->mode = !aaultra_sql->mode;
        break;
    case 2:
        list << BASIC << Spectra;
        Common::change_index(aaultra_sql->chart, d, list);
        break;
    case 3:
        if( (aaultra_sql->gain<9.95 && d>0) || (aaultra_sql->gain<10.15 && d<0) ){
            Common::change_index(aaultra_sql->gain, d * 0.1, 100, 0.1 );
        }
        else{
            Common::change_index(aaultra_sql->gain, d * 10, 100, 0.1 );
        }
        break;
    case 4:
        Common::change_index(aaultra_sql->vol, d, VOL_MAX, VOL_MIN );
        break;
    case 5:
        Common::change_index(aaultra_sql->low, d, aaultra_sql->high, 0 );
        break;
    case 6:
        Common::change_index(aaultra_sql->high, d, 60, aaultra_sql->low );
        break;
    case 7:
        Common::change_index(aaultra_sql->time, d, TIME_MAX, TIME_MIN );
        break;
    default:
        break;
    }
}

void AAWidget::fresh(bool f)
{
    int offset;
    double val,val_db;

    Common::calc_aa_value(data,mode,aaultra_sql,&val, &val_db, &offset);

    if(db < int(val_db)){
        db = int(val_db);      //每秒的最大值
    }

    //记录并显示最大值
    if (max_db <val_db ) {
        max_db = val_db;
        ui->label_max->setText(tr("最大值: ") + QString::number(max_db));
    }

    if(f){  //直接显示（1s一次）
        ui->label_val->setText(QString::number(val_db, 'f', 1));
        temp_db = val_db;
        //彩色显示
        if ( val_db >= aaultra_sql->high) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
            emit beep(menu_index, 2);        //蜂鸣器报警
        } else if (val_db >= aaultra_sql->low) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
            emit beep(menu_index, 1);
        } else {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
        }
//        #if 0

        int is_current = 0;
        if((int)key_val->grade.val0 == menu_index){
            is_current = 1;
        }

        if(mode == AA1){
            Common::rdb_set_yc_value(AA1_amplitude,val_db,is_current);
            Common::rdb_set_yc_value(AA1_severity,0,is_current);
            Common::rdb_set_yc_value(AA1_gain,aaultra_sql->gain,is_current);
            Common::rdb_set_yc_value(AA1_biased,aaultra_sql->offset,is_current);
            Common::rdb_set_yc_value(AA1_biased_adv,offset,is_current);
        }
        else if(mode == AA2){
            Common::rdb_set_yc_value(AA2_amplitude,val_db,is_current);
            Common::rdb_set_yc_value(AA2_severity,0,is_current);
            Common::rdb_set_yc_value(AA2_gain,aaultra_sql->gain,is_current);
            Common::rdb_set_yc_value(AA2_biased,aaultra_sql->offset,is_current);
            Common::rdb_set_yc_value(AA2_biased_adv,offset,is_current);
        }
        emit send_log_data(val_db,0,0,is_current);
//        #endif
    }
    else{   //条件显示
        if(qAbs(val_db-temp_db ) >= aaultra_sql->step){
            ui->label_val->setText(QString::number(val_db, 'f', 1));
            if ( val_db > aaultra_sql->high) {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
            } else if (val_db >= aaultra_sql->low) {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
            } else {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
            }
        }
    }

    double max_val_range;
    if (val < 1) {
        max_val_range = 1;
        ui->label_range->setText("μV");                                                //range fresh
    } else if (val < 10) {
        max_val_range = 10;
        ui->label_range->setText("10μV");                                              //range fresh
    } else if (val < 100) {
        max_val_range = 100;
        ui->label_range->setText("100μV");                                              //range fresh
    } else if (val < 1000) {
        max_val_range = 1000;
        ui->label_range->setText("mV");                                                 //range fresh
    } else {
        max_val_range = 10000;
        ui->label_range->setText("10mV");                                               //range fresh
    }
    ui->progressBar->setValue(val * 100 / max_val_range);


}

void AAWidget::fresh_1000ms()
{
    fresh(true);
    d_BarChart->fresh();
    ui->qwtPlot->replot();
}

void AAWidget::fresh_100ms()
{
    fresh(false);
//    if(aaultra_sql->chart == Spectra){
            do_Spectra_compute();
//    }
}

void AAWidget::fresh_setting()
{
    if (aaultra_sql->mode == single) {
        ui->comboBox->setItemText(0,tr("检测模式\t[单次]"));
        timer_100ms->setSingleShot(true);
        timer_1000ms->setSingleShot(true);
    } else {
        ui->comboBox->setItemText(0,tr("检测模式\t[连续]"));
        timer_100ms->setSingleShot(false);
        timer_1000ms->setSingleShot(false);
    }

    ui->qwtPlot->hide();
    plot_Spectra->hide();

    switch (aaultra_sql->chart) {
    case BASIC:
        ui->qwtPlot->show();
        ui->comboBox->setItemText(1,tr("图形显示 \t[时序图]"));
        break;
    case Spectra:
        plot_Spectra->show();
        ui->comboBox->setItemText(1,tr("图形显示  \t[频谱图]"));
        break;
    default:
        break;
    }

    ui->comboBox->setItemText(2,tr("增益调节\t[×%1]").arg(QString::number(aaultra_sql->gain, 'f', 1)) );
    ui->comboBox->setItemText(3,tr("音量调节\t[×%1]").arg(QString::number(aaultra_sql->vol)));
    ui->comboBox->setItemText(4,tr("黄色报警阈值\t[%1]dB").arg(QString::number(aaultra_sql->low)));
    ui->comboBox->setItemText(5,tr("红色报警阈值\t[%1]dB").arg(QString::number(aaultra_sql->high)));
    ui->comboBox->setItemText(6,tr("连续录波\t[%1]s").arg(aaultra_sql->time));

    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == menu_index && key_val->grade.val5 == 0) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    emit fresh_parent();
}



