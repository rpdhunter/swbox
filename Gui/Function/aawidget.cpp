#include "aawidget.h"
#include "ui_aawidget.h"
#include <QLineEdit>
#include <QTimer>
#include "IO/Com/rdb/rdb.h"
#include "Gui/Common/fft.h"

#define SETTING_NUM 8           //设置菜单条目数
#define VALUE_MAX 60

AAWidget::AAWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::AAWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("AAWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    Common::set_comboBox_style(ui->comboBox);

    this->data = data;
    this->key_val = val;
    this->mode = mode;
    this->menu_index = menu_index;

    reload(-1);
    fft = new FFT;

    temp_db = 0;
    db = 0;
    isBusy = false;

    //图形设置
    Common::set_barchart_style(ui->qwtPlot, VALUE_MAX);    
    chart = new BarChart(ui->qwtPlot, &db, &aaultra_sql->high, &aaultra_sql->low);
    chart->resize(200, 140);

    //Spectra
    plot_Spectra = new QwtPlot(ui->widget);
    plot_Spectra->resize(200, 140);
    d_Spectra = new QwtPlotHistogram;
    Common::set_Spectra_style(plot_Spectra,d_Spectra,0,4,0,30,"");

    timer_100ms = new QTimer(this);
    timer_100ms->setInterval(100);
    connect(timer_100ms, SIGNAL(timeout()), this, SLOT(fresh_100ms()));   //每0.1秒刷新一次数据状态，明显的变化需要快速显示

    timer_1000ms = new QTimer(this);
    timer_1000ms->setInterval(1000);
    connect(timer_1000ms, SIGNAL(timeout()), this, SLOT(fresh_1000ms()));   //每1秒刷新一次数据状态

    recWaveForm = new RecWaveForm(menu_index,this);
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));
    connect(recWaveForm,SIGNAL(fresh_parent()),this,SIGNAL(fresh_parent()));

    logtools = new LogTools(mode);      //日志保存模块
    connect(this,SIGNAL(aa_log_data(double,int,double)),logtools,SLOT(dealLog(double,int,double)));

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
        ae_pulse_data = &data->recv_para_ae1;
    }
    else if(mode == AA2){
        aaultra_sql = &sql_para.aa2_sql;
        ae_pulse_data = &data->recv_para_ae1;
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

void AAWidget::trans_key(quint8 key_code)
{    
    if (key_val == NULL || key_val->grade.val0 != menu_index) {
        return;
    }

    if(isBusy){
        return;
    }

    if(key_val->grade.val5 != 0){
        emit send_key(key_code);
        return;
    }
//    qDebug()<<"val0 = "<<key_val->grade.val0 <<"\nval1 = "<<key_val->grade.val1 <<"\nval2 = "<<key_val->grade.val2 ;
    switch (key_code) {
    case KEY_OK:
        sqlcfg->sql_save(&sql_para);
        reload(menu_index);
        switch (key_val->grade.val2) {
        case 7:
            key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
            emit startRecWave(mode, aaultra_sql->time);        //发送录波信号
            emit show_indicator(true);
            isBusy = true;
            return;
        case 8:
            this->maxReset();
            break;
        default:
            break;
        }
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_CANCEL:
        reload(-1);        //重置默认数据
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_UP:
        do_key_up_down(-1);
        break;
    case KEY_DOWN:
        do_key_up_down(1);
        break;
    case KEY_LEFT:
        do_key_left_right(-1);
        break;
    case KEY_RIGHT:
        do_key_left_right(1);
        break;
    default:
        break;
    }
    emit fresh_parent();
    fresh_setting();
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

void AAWidget::do_Spectra_compute()
{
    if(ae_datalist.count() > 2048){
        QVector<qint32> fft_result = fft->fft2048(ae_datalist.mid(0,2048));
        for (int i = 0; i <41 ; ++i) {
            Spectra_map[i] = Common::avrage(fft_result.mid(i*5 + 1, (i+1)*5) );
        }

        Spectra_data.clear();
        for(int i=0;i<40;i++){
            QwtInterval interval( 0.1*(i + 0.2) , 0.1*(i + 0.8) );
            interval.setBorderFlags( QwtInterval::ExcludeMaximum );
            Spectra_data.append( QwtIntervalSample( Spectra_map[i], interval ) );
        }

        d_Spectra->setData(new QwtIntervalSeriesData( Spectra_data ));
        plot_Spectra->replot();
        ae_datalist.clear();
    }
}

void AAWidget::showWaveData(VectorList buf, MODE mod)
{
    if(key_val->grade.val0 == menu_index){       //在超声界面，可以显示
        isBusy = false;
        emit show_indicator(false);
        ui->comboBox->hidePopup();
        key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
        key_val->grade.val5 = 1;
        emit fresh_parent();
        ui->comboBox->hidePopup();
        recWaveForm->working(key_val,buf,mod);
    }
}

void AAWidget::change_log_dir()
{
    logtools->change_current_asset_dir();
}

void AAWidget::add_ae_data()
{
    for (int i = 0; i < 128; ++i) {
        ae_datalist.append(ae_pulse_data->data[i+2]);
    }

    ae_timelist.append(ae_pulse_data->time/* + ae_timelist.last()*/);

    ae_pulse_data->readComplete = 1;        //读取完成标志
}

void AAWidget::save_channel()
{

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
        emit aa_log_data(val_db,0,0);

        yc_data_type temp_data;
        if(mode == AA1){
            temp_data.f_val = val_db;
            yc_set_value(AA1_amplitude, &temp_data, 0, NULL,0);
            temp_data.f_val = 0;
            yc_set_value(AA1_severity, &temp_data, 0, NULL,0);
            temp_data.f_val = aaultra_sql->gain;
            yc_set_value(AA1_gain, &temp_data, 0, NULL,0);
            temp_data.f_val = aaultra_sql->offset;
            yc_set_value(AA1_biased, &temp_data, 0, NULL,0);
            temp_data.f_val = offset;
            yc_set_value(AA1_biased_adv, &temp_data, 0, NULL,0);
        }
        else if(mode == AA2){
            temp_data.f_val = val_db;
            yc_set_value(AA2_amplitude, &temp_data, 0, NULL,0);
            temp_data.f_val = 0;
            yc_set_value(AA2_severity, &temp_data, 0, NULL,0);
            temp_data.f_val = aaultra_sql->gain;
            yc_set_value(AA2_gain, &temp_data, 0, NULL,0);
            temp_data.f_val = aaultra_sql->offset;
            yc_set_value(AA2_biased, &temp_data, 0, NULL,0);
            temp_data.f_val = offset;
            yc_set_value(AA2_biased_adv, &temp_data, 0, NULL,0);
        }

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
            yc_data_type temp_data;
            temp_data.f_val = val_db;
            yc_set_value(AA1_amplitude, &temp_data, 0, NULL,0);
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

//    ui->label_range->setText(QString("%1").arg(data->recv_para.ldata1_max * 4 * aaultra_sql->gain * AA_FACTOR));
}

void AAWidget::fresh_1000ms()
{
    fresh(true);
    chart->fresh();
    ui->qwtPlot->replot();
}

void AAWidget::fresh_100ms()
{
    fresh(false);
    do_Spectra_compute();
}

void AAWidget::maxReset()
{
    max_db = 0;
    ui->label_max->setText(tr("最大值: ") + QString::number(max_db));
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
}


