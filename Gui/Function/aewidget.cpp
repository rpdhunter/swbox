#include "aewidget.h"
#include "ui_aewidget.h"

#include <QLineEdit>
#include <QTimer>
#include "IO/Com/rdb/rdb.h"

#define SETTING_NUM 9           //设置菜单条目数
#define VALUE_MAX 60
#define TEST

AEWidget::AEWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::AEWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("AEWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");    
    Common::set_comboBox_style(ui->comboBox);

    this->data = data;
    this->key_val = val;
    this->mode = mode;
    this->menu_index = menu_index;

    reload(-1);
    temp_db = 0;
    db = 0;
    isBusy = false;

    //图形设置
    chart_ini();


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
    connect(this,SIGNAL(ae_log_data(double,int,double)),logtools,SLOT(dealLog(double,int,double)));
    connect(this,SIGNAL(ae_PRPD_data(QVector<QwtPoint3D>)),logtools,SLOT(dealRPRDLog(QVector<QwtPoint3D>)));
//    reload(menu_index);
}

AEWidget::~AEWidget()
{
    delete ui;
}

void AEWidget::add_ae_data()
{
    for (int i = 0; i < 128; ++i) {
        ae_datalist.append(ae_pulse_data->data[i+2]);
    }

    ae_timelist.append(ae_pulse_data->time/* + ae_timelist.last()*/);

    ae_pulse_data->readComplete = 1;        //读取完成标志
}

void AEWidget::fresh_1000ms()
{
    fresh(true);
    fresh_Histogram();
    d_BarChart->fresh();
    plot_Barchart->replot();
    plot_PRPD->replot();
    plot_Histogram->replot();

#ifdef TEST
    qDebug()<<QTime::currentTime();

    //数据量显示
    qDebug()<<"ae_datalist\t"<<ae_datalist.count() << "\t"<< ae_datalist.count()/128.0;

    //数据显示
//    qDebug()<<ae_datalist.mid(0,100);

    //时标显示
    for (int i = ae_timelist.count()-1; i > 0; i--) {
        ae_timelist[i] = ae_timelist.at(i) - ae_timelist.at(i-1);
    }
    qDebug()<<"ae_timelist\t"<<ae_timelist.count() << "\t"<< ae_timelist;

    ae_timelist.clear();
    ae_datalist.clear();
#endif

}

void AEWidget::fresh_100ms()
{
    fresh(false);

//    qDebug()<<"ae_datalist\t"<<ae_datalist.count() << "\t"<< ae_datalist.count()/128.0;
#ifndef TEST
    QVector<QPoint> pulse_100ms = Common::calc_pulse_list(ae_datalist,ae_timelist,aeultra_sql->fpga_threshold);
//    foreach (QPoint p, pulse_100ms) {
//        qDebug()<<"all"<<p;
//    }

    int x,y;
    double _y;
    for(int i=0; i<pulse_100ms.count(); i++){
        x = Common::time_to_phase(pulse_100ms.at(i).x() );              //时标
        _y = Common::physical_value(pulse_100ms.at(i).y(),mode);         //强度
        y = (int)20*log(qAbs(_y) );


        if(x<360 && x>=0 && y<=60 &&y>=-60){
            QwtPoint3D p0(x,y,map[x][y+60]);
            map[x][y+60]++;
            QwtPoint3D p1(x,y,map[x][y+60]);
            if(map[x][y+60]>1){
                int n = prpd_samples.indexOf(p0);
                prpd_samples[n] = p1;
            }
            else{
                prpd_samples.append(p1);
            }
        }
        else{
            qDebug()<<QPointF(x,y) << "\t"<< pulse_100ms.at(i);
        }
    }

    d_PRPD->setSamples(prpd_samples);
    plot_PRPD->replot();

    ae_timelist.clear();
    ae_datalist.clear();
#endif

}

void AEWidget::reload(int index)
{
    //基本sql内容的初始化
    sql_para = *sqlcfg->get_para();
    if(mode == AE1){
        aeultra_sql = &sql_para.ae1_sql;
        ae_pulse_data = &data->recv_para_ae1;
    }
    else if(mode == AE2){
        aeultra_sql = &sql_para.ae2_sql;
        ae_pulse_data = &data->recv_para_ae2;
    }

    //构造函数中计时器不启动
    if(index == menu_index){
        if(!timer_100ms->isActive()){
            timer_100ms->start();
        }
        if(!timer_1000ms->isActive()){
            timer_1000ms->start();
        }

        if(mode == AE1){
            data->set_send_para (sp_vol_l1, aeultra_sql->vol);
            data->set_send_para (sp_aa_record_play, 0);        //耳机送1通道
        }
        else if(mode == AE2){
            data->set_send_para (sp_vol_l2, aeultra_sql->vol);
            data->set_send_para (sp_aa_record_play, 2);        //耳机送2通道
        }
        data->set_send_para(sp_auto_rec, 0);        //关闭自动录波
        ae_pulse_data->readComplete = 1;        //读取完成标志
        fresh_setting();
    }
}

void AEWidget::trans_key(quint8 key_code)
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
        case 8:
            key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
            emit startRecWave(mode, aeultra_sql->time);        //发送录波信号
            emit show_indicator(true);
            isBusy = true;
            return;
        case 9:
            maxReset();
            PRPDReset();
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

void AEWidget::do_key_up_down(int d)
{
    key_val->grade.val1 = 1;
    Common::change_index(key_val->grade.val2, d, SETTING_NUM, 1);
}

void AEWidget::do_key_left_right(int d)
{
    QList<int> list;
    switch (key_val->grade.val2) {
    case 1:
        aeultra_sql->mode = !aeultra_sql->mode;
        break;
    case 2:
        list << BASIC << PRPD << FLY << Exponent;
        Common::change_index(aeultra_sql->chart, d, list);
        break;
    case 3:
        Common::change_index(aeultra_sql->gain, d * 0.1, 20, 0.1 );
        break;
    case 4:
        Common::change_index(aeultra_sql->vol, d, VOL_MAX, VOL_MIN );
        break;
    case 5:
        Common::change_index(aeultra_sql->low, d, aeultra_sql->high, 0 );
        break;
    case 6:
        Common::change_index(aeultra_sql->high, d, 60, aeultra_sql->low );
        break;
    case 7:
        aeultra_sql->fpga_threshold += Common::code_value(1,mode) * d;
        break;
    case 8:
        Common::change_index(aeultra_sql->time, d, TIME_MAX, TIME_MIN );
        break;
    default:
        break;
    }
}

void AEWidget::chart_ini()
{
    //barchart
    plot_Barchart = new QwtPlot(ui->widget);
    Common::set_barchart_style(plot_Barchart, VALUE_MAX);
    plot_Barchart->resize(200, 140);
    d_BarChart = new BarChart(plot_Barchart, &db, &aeultra_sql->high, &aeultra_sql->low);

    //PRPD
    plot_PRPD = new QwtPlot(ui->widget);
    plot_PRPD->resize(200, 140);
    d_PRPD = new QwtPlotSpectroCurve;
    Common::set_PRPD_style(plot_PRPD,d_PRPD,VALUE_MAX);
    PRPDReset();

    //histogram
    plot_Histogram = new QwtPlot(ui->widget);
    plot_Histogram->resize(200, 140);
    d_histogram = new QwtPlotHistogram;
    Common::set_histogram_style(plot_Histogram,d_histogram);
//    plot_Histogram->setAxisScale(QwtPlot::xBottom, 0, 6);
//    plot_Histogram->setAxisScale(QwtPlot::yLeft, 0, 50);
//    plot_Histogram->axisWidget(QwtPlot::xBottom)->setTitle("");

}

void AEWidget::PRPDReset()
{
    for(int i=0;i<360;i++){
        for(int j=0;j<121;j++){
            map[i][j]=0;
        }
    }
    emit ae_PRPD_data(prpd_samples);
    prpd_samples.clear();
}

void AEWidget::fresh_Histogram()
{
    histogram_data.clear();

    int tmp;

    for(int j=0;j<121;j++){
        tmp = 0;
        for(int i=0;i<360;i++){
            tmp += map[i][j];
        }
        QwtInterval interval( j - 60.0 , j - 59.0 );
        interval.setBorderFlags( QwtInterval::ExcludeMaximum );
        histogram_data.append( QwtIntervalSample( tmp, interval ) );
    }

    d_histogram->setData(new QwtIntervalSeriesData( histogram_data ));

    plot_Histogram->replot();
}

void AEWidget::showWaveData(VectorList buf, MODE mod)
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

void AEWidget::fresh(bool f)
{
    int offset;
    double val,val_db;

    Common::calc_aa_value(data,mode,aeultra_sql,&val, &val_db, &offset);


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
        if ( val_db >= aeultra_sql->high) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
            emit beep(menu_index, 2);        //蜂鸣器报警
        } else if (val_db >= aeultra_sql->low) {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
            emit beep(menu_index, 1);
        } else {
            ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
        }
        emit ae_log_data(val_db,0,0);

        yc_data_type temp_data;
        if(mode == AE1){
            temp_data.f_val = val_db;
            yc_set_value(AE1_amplitude, &temp_data, 0, NULL,0);
            temp_data.f_val = 0;
            yc_set_value(AE1_severity, &temp_data, 0, NULL,0);
            temp_data.f_val = aeultra_sql->gain;
            yc_set_value(AE1_gain, &temp_data, 0, NULL,0);
            temp_data.f_val = aeultra_sql->offset;
            yc_set_value(AE1_biased, &temp_data, 0, NULL,0);
            temp_data.f_val = offset;
            yc_set_value(AE1_biased_adv, &temp_data, 0, NULL,0);
        }
        else if(mode == AE2){
            temp_data.f_val = val_db;
            yc_set_value(AE2_amplitude, &temp_data, 0, NULL,0);
            temp_data.f_val = 0;
            yc_set_value(AE2_severity, &temp_data, 0, NULL,0);
            temp_data.f_val = aeultra_sql->gain;
            yc_set_value(AE2_gain, &temp_data, 0, NULL,0);
            temp_data.f_val = aeultra_sql->offset;
            yc_set_value(AE2_biased, &temp_data, 0, NULL,0);
            temp_data.f_val = offset;
            yc_set_value(AE2_biased_adv, &temp_data, 0, NULL,0);
        }

    }
    else{   //条件显示
        if(qAbs(val_db-temp_db ) >= aeultra_sql->step){
            ui->label_val->setText(QString::number(val_db, 'f', 1));
            if ( val_db > aeultra_sql->high) {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
            } else if (val_db >= aeultra_sql->low) {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
            } else {
                ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
            }
            yc_data_type temp_data;
            temp_data.f_val = val_db;
            yc_set_value(AE1_amplitude, &temp_data, 0, NULL,0);
        }
    }

}


void AEWidget::maxReset()
{
    max_db = 0;
    ui->label_max->setText(tr("最大值: ") + QString::number(max_db));
}

void AEWidget::fresh_setting()
{
    if (aeultra_sql->mode == single) {
        ui->comboBox->setItemText(0,tr("检测模式\t[单次]"));
        timer_100ms->setSingleShot(true);
        timer_1000ms->setSingleShot(true);
    } else {
        ui->comboBox->setItemText(0,tr("检测模式\t[连续]"));
        timer_100ms->setSingleShot(false);
        timer_1000ms->setSingleShot(false);
    }

    if (aeultra_sql->chart == PRPD) {
        ui->comboBox->setItemText(1,tr("图形显示\t[PRPD]"));
        plot_PRPD->show();
        plot_Barchart->hide();
        plot_Histogram->hide();

    } else if(aeultra_sql->chart == BASIC){
        ui->comboBox->setItemText(1,tr("图形显示 \t[时序图]"));
        plot_PRPD->hide();
        plot_Barchart->show();
        plot_Histogram->hide();

    } else if(aeultra_sql->chart == FLY){
        ui->comboBox->setItemText(1,tr("图形显示 \t[飞行图]"));
        plot_PRPD->hide();
        plot_Barchart->hide();
        plot_Histogram->hide();

    } else if(aeultra_sql->chart == Exponent){
        ui->comboBox->setItemText(1,tr("图形显示  [特征指数]"));
        plot_PRPD->hide();
        plot_Barchart->hide();
        plot_Histogram->show();

    }
    ui->comboBox->setItemText(2,tr("增益调节\t[×%1]").arg(QString::number(aeultra_sql->gain, 'f', 1)) );
    ui->comboBox->setItemText(3,tr("音量调节\t[×%1]").arg(QString::number(aeultra_sql->vol)));
    ui->comboBox->setItemText(4,tr("黄色报警阈值\t[%1]dB").arg(QString::number(aeultra_sql->low)));
    ui->comboBox->setItemText(5,tr("红色报警阈值\t[%1]dB").arg(QString::number(aeultra_sql->high)));
    ui->comboBox->setItemText(6,tr("脉冲触发\t[%1]mV").arg(QString::number((int)Common::physical_value(aeultra_sql->fpga_threshold,mode) )));
    ui->comboBox->setItemText(7,tr("连续录波\t[%1]s").arg(aeultra_sql->time));

    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == menu_index && key_val->grade.val5 == 0) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
}
