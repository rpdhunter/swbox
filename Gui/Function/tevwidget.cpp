#include "tevwidget.h"
#include "ui_tevwidget.h"
#include <QTimer>
#include <QLineEdit>
#include "IO/Other/buzzer.h"

#define VALUE_MAX  60           //RPPD最大值
#define SETTING_NUM 9           //设置菜单条目数
#define TOKEN_MAX   50          //最大允许一次处理的数据量


TEVWidget::TEVWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TEVWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("TEVWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    Common::set_comboBox_style(ui->comboBox);

    this->data = data;
    this->key_val = val;
    this->mode = mode;
    this->menu_index = menu_index;

    reload(-1);

    db = 0;
    max_db = 0;
    pulse_cnt_last = 0;
    pulse_number = 0;
    db_last1 = 0;
    db_last2 = 0;
    manual = false;
    token = 0;


    timer_1000ms = new QTimer(this);
    timer_1000ms->setInterval(1000);
    connect(timer_1000ms, SIGNAL(timeout()), this, SLOT(fresh_1000ms()));
//    connect(timer_1000ms, SIGNAL(timeout()), this, SLOT(fresh_plot()));
//    connect(timer_1000ms, SIGNAL(timeout()), this, SLOT(fresh_Histogram()));
//    connect(timer_1000ms, SIGNAL(timeout()), d_BarChart, SLOT(fresh()) );

    timer_1ms = new QTimer(this);
    timer_1ms->setInterval(2);         //1ms读取一次数据
    connect(timer_1ms, SIGNAL(timeout()), this, SLOT(add_token()));

    timer_200ms = new QTimer(this);
    timer_200ms->setInterval(200);
    connect(timer_200ms, SIGNAL(timeout()), this, SLOT(fresh_200ms()));
//    connect(timer_200ms, SIGNAL(timeout()), this, SLOT(fresh_PRPD()));

    timer_freeze = new QTimer(this);      //timer3设置了一个界面手动退出后的锁定期,便于操作
    timer_freeze->setInterval(FREEZE_TIME);      //5秒内不出现新录波界面
    timer_freeze->setSingleShot(true);

    chart_ini();

    recWaveForm = new RecWaveForm(menu_index,this);
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));
    connect(recWaveForm,SIGNAL(fresh_parent()),this,SIGNAL(fresh_parent()));
    connect(recWaveForm, SIGNAL(fresh_parent()), timer_freeze, SLOT(start()) );

    logtools = new LogTools(mode);      //日志保存模块
    connect(this,SIGNAL(tev_log_data(double,int,double)),logtools,SLOT(dealLog(double,int,double)));
    connect(this,SIGNAL(tev_PRPD_data(QVector<QwtPoint3D>)),logtools,SLOT(dealRPRDLog(QVector<QwtPoint3D>)));

    reload(menu_index);

//    if(mode == TEV1){
//        Buzzer *buzzer = new Buzzer(data);
//        buzzer->start();
//    }
}

TEVWidget::~TEVWidget()
{
    delete ui;
}

void TEVWidget::reload(int index)
{
    //基本sql内容的初始化
    sql_para = *sqlcfg->get_para();
    if(mode == TEV1){
        tev_sql = &sql_para.tev1_sql;
        short_data = &data->recv_para_short1;
    }
    else if(mode == TEV2){
        tev_sql = &sql_para.tev2_sql;
        short_data = &data->recv_para_short2;
    }

    //仅切换到此界面的初始化
    if(index == menu_index){
        //条件启动计时器
        if(!timer_1000ms->isActive()){
            timer_1000ms->start();
        }
        if(!timer_1ms->isActive()){
            timer_1ms->start();
        }
        if(!timer_200ms->isActive()){
            timer_200ms->start();
        }
        //自动录波
        if(tev_sql->auto_rec == true){
            data->set_send_para(sp_auto_rec, menu_index + 1);
        }
        else{
            data->set_send_para(sp_auto_rec, 0);
        }
        fresh_setting();
    }
}

void TEVWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL || key_val->grade.val0 != menu_index) {
        return;
    }
    //    qDebug()<<"TEV\tval0 = "<<key_val->grade.val0 <<"\tval1 = "<<key_val->grade.val1 <<"\tval2 = "<<key_val->grade.val2 ;

    if(key_val->grade.val5 != 0){
        emit send_key(key_code);
        return;
    }

    switch (key_code) {
    case KEY_OK:
        sqlcfg->sql_save(&sql_para);
        reload(menu_index);        //重置默认数据
        switch (key_val->grade.val2) {
        case 6:
            //自动录波
            if(tev_sql->auto_rec == true){
                data->set_send_para (sp_rec_on, 1);
                data->set_send_para(sp_auto_rec, menu_index + 1);
            }
            else{
                data->set_send_para (sp_rec_on, 0);
                data->set_send_para(sp_auto_rec, 0);
            }
            break;
        case 7:
            emit startRecWave(mode,0);     //开始录波
            manual = true;
            break;
        case 8:
            maxReset();
            break;
        case 9:
            PRPDReset();
            break;
        default:
            break;
        }
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_CANCEL:
        reload(menu_index);        //重置默认数据
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
    fresh_setting();
}

void TEVWidget::do_key_up_down(int d)
{
    key_val->grade.val1 = 1;
    Common::change_index(key_val->grade.val2,d,SETTING_NUM,1);
}

void TEVWidget::do_key_left_right(int d)
{
    switch (key_val->grade.val2) {
    case 1:
        tev_sql->mode = !tev_sql->mode;
        break;
    case 2:
        Common::change_index(tev_sql->mode_chart,d,Histogram,BASIC);
        break;
    case 3:
        Common::change_index(tev_sql->gain, d * 0.1, 20, 0.1 );
        break;
    case 4:
        Common::change_index(tev_sql->low, d, tev_sql->high, 0 );
        break;
    case 5:
        Common::change_index(tev_sql->high, d, 60, tev_sql->low );
        break;
    case 6:
        tev_sql->auto_rec = !tev_sql->auto_rec;
        break;
    default:
        break;
    }
}

void TEVWidget::showWaveData(VectorList buf, MODE mod)
{
    if( (key_val->grade.val0 == menu_index ) && ( !timer_freeze->isActive() || !tev_sql->auto_rec || manual == true) ){
        key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
        key_val->grade.val5 = 1;
        emit fresh_parent();
        ui->comboBox->hidePopup();
        manual = false;
        recWaveForm->working(key_val,buf,mod);
    }
}

void TEVWidget::chart_ini()
{
    //barchart
    plot_Barchart = new QwtPlot(ui->widget);
    Common::set_barchart_style(plot_Barchart, VALUE_MAX);
    plot_Barchart->resize(200, 140);
    d_BarChart = new BarChart(plot_Barchart, &db, &tev_sql->high, &tev_sql->low);

    //PRPS
    scene = new PRPSScene(mode,VALUE_MAX);
    plot_PRPS = new QGraphicsView(ui->widget);
    plot_PRPS->resize(ui->widget->size());
    plot_PRPS->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    plot_PRPS->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    plot_PRPS->setStyleSheet("background:transparent;color:gray;");
    plot_PRPS->setScene(scene);

    //PRPD
//    if(mode == TEV1){
//        data_prpd = &data->recv_para_prpd1;
//    }
//    else if(mode == TEV2){
//        data_prpd = &data->recv_para_prpd2;
//    }
    plot_PRPD = new QwtPlot(ui->widget);
    plot_PRPD->resize(200, 140);
    d_PRPD = new QwtPlotSpectroCurve;
    Common::set_PRPD_style(plot_PRPD,d_PRPD,VALUE_MAX);
    PRPDReset();

    //histogram
    plot_Histogram = new QwtPlot(ui->widget);
    plot_Histogram->resize(200, 150);
    d_histogram = new QwtPlotHistogram;
    Common::set_histogram_style(plot_Histogram,d_histogram);


}

void TEVWidget::calc_tev_value (double * tev_val, double * tev_db, int * sug_central_offset, int * sug_offset)
{
    int d_max, d_min, a, b;
    double db;

    if (mode == TEV1) {
        d_max = data->recv_para_normal.hdata0.ad.ad_max;
        d_min = data->recv_para_normal.hdata0.ad.ad_min;
    }
    else{
        d_max = data->recv_para_normal.hdata1.ad.ad_max;
        d_min = data->recv_para_normal.hdata1.ad.ad_min;
    }

    * sug_central_offset = ((d_max + d_min) >> 1) - 0x8000;
    //	* sug_offset = ((d_max - d_min) >> 1) / 10;

    a = d_max - 0x8000 - tev_sql->fpga_zero;        //减去中心偏置
    b = d_min - 0x8000 - tev_sql->fpga_zero;        //减去中心偏置

    * sug_offset = ( MAX (qAbs (a), qAbs (b)) - 1 / TEV_FACTOR / tev_sql->gain ) /10;

    db = tev_sql->gain * (MAX (qAbs (a), qAbs (b)) - tev_sql->tev_offset1 * 10) * TEV_FACTOR;
    * tev_val = db;



    db = 20 * log10 (db);      //对数运算，来自工具链的函数

//    qDebug()<<"TEV original value : " << * tev_val << "\tTEV db value"<< db;

    if(db < 0){
        db = 0;
    }
    * tev_db = db;
}

void TEVWidget::fresh_plot()
{
    double t, s, degree;
    quint32 pulse_cnt,pulse_cnt_show;      //脉冲计数
    int sug_central_offset, sug_offset;
    //    quint32 signal_pulse_cnt;

    //    if (tev_sql->mode == single) {
    //        timer1->stop();      //如果单次模式，停止计时器
    //    }

    calc_tev_value (&t, &s, &sug_central_offset, &sug_offset);

//    qDebug()<<data->recv_para_normal.hpulse1_totol;

    //记录并显示最大值
    if (max_db < s) {
        max_db = s;
        ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "dB");
    }

    //脉冲计数和严重度
    //    signal_pulse_cnt = data->recv_para.pulse1.edge.neg + data->recv_para.pulse1.edge.pos;
    if(mode == TEV1){
        pulse_cnt = data->recv_para_normal.hpulse0_totol;
    }
    else{
        pulse_cnt = data->recv_para_normal.hpulse1_totol;
//        qDebug()<<"changgui = "<<pulse_cnt << "\tjisuan = "<< pulse_number;

    }


    pulse_cnt_show = pulse_cnt_last + pulse_cnt;    //显示2秒的脉冲计数
    pulse_cnt_last = pulse_cnt;

    degree = pow(t,tev_sql->gain) * (double)pulse_cnt / sql_para.freq_val;     //严重度算法更改

    ui->label_pluse->setText(tr("脉冲数: ") + QString::number(pulse_number));
    ui->label_degree->setText(tr("严重度: ") + QString::number(degree, 'f', 2));



    if(qAbs(s - db_last1) <15){
        if(db_last1 > 16){
            db = (int)MAX(db_last1,s);
        }
        else{
            db = (int)MIN(db_last1,s);
        }
    }
    else{
        db = (int)s;
        db_last2 = db_last1;
    }
    db_last1 = s;
#if 0
    //db值小于20时, 逐渐递减, 越接近0, 递减幅度越大
    if(db < 20){
        db = db * (0.5 + db / 20.0 );
    }
#endif
    if(db > 60){
        db = 60;
    }

    ui->label_val->setText(QString::number((qint16)s));

    db = (int)s;

    emit tev_log_data(s,pulse_cnt_show,degree);


    yc_data_type temp_data;
    if(mode == TEV1){
        temp_data.f_val = db;
        yc_set_value(TEV1_amplitude, &temp_data, 0, NULL,0);
        temp_data.f_val = pulse_number;
        yc_set_value(TEV1_num, &temp_data, 0, NULL,0);
        temp_data.f_val = degree;
        yc_set_value(TEV1_severity, &temp_data, 0, NULL,0);
        temp_data.f_val = tev_sql->gain;
        yc_set_value(TEV1_gain, &temp_data, 0, NULL,0);
        temp_data.f_val = tev_sql->fpga_zero;
        yc_set_value(TEV1_center_biased, &temp_data, 0, NULL,0);
        temp_data.f_val = tev_sql->tev_offset1;
        yc_set_value(TEV1_noise_biased, &temp_data, 0, NULL,0);
        temp_data.f_val = sug_central_offset;
        yc_set_value(TEV1_center_biased_adv, &temp_data, 0, NULL,0);
        temp_data.f_val = sug_offset;
        yc_set_value(TEV1_noise_biased_adv, &temp_data, 0, NULL,0);
    }
    else{
        temp_data.f_val = db;
        yc_set_value(TEV2_amplitude, &temp_data, 0, NULL,0);
        temp_data.f_val = pulse_cnt_show;
        yc_set_value(TEV2_num, &temp_data, 0, NULL,0);
        temp_data.f_val = degree;
        yc_set_value(TEV2_severity, &temp_data, 0, NULL,0);
        temp_data.f_val = tev_sql->gain;
        yc_set_value(TEV2_gain, &temp_data, 0, NULL,0);
        temp_data.f_val = tev_sql->fpga_zero;
        yc_set_value(TEV2_center_biased, &temp_data, 0, NULL,0);
        temp_data.f_val = tev_sql->tev_offset1;
        yc_set_value(TEV2_noise_biased, &temp_data, 0, NULL,0);
        temp_data.f_val = sug_central_offset;
        yc_set_value(TEV2_center_biased_adv, &temp_data, 0, NULL,0);
        temp_data.f_val = sug_offset;
        yc_set_value(TEV2_noise_biased_adv, &temp_data, 0, NULL,0);
    }

    if ( db >= tev_sql->high) {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
        emit beep(menu_index,2);        //蜂鸣器报警
    } else if (db >= tev_sql->low) {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
        emit beep(menu_index,1);
    } else {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
    }

    pulse_number = 0;

//    if (tev_sql->mode_chart == PRPD) {
//        plot_PRPD->show();
//        plot_Barchart->hide();
//        plot_Histogram->hide();
        plot_PRPD->replot();
//    } else if(tev_sql->mode_chart == BASIC){
//        plot_PRPD->hide();
//        plot_Barchart->show();
//        plot_Histogram->hide();
        plot_Barchart->replot();
//    } else if(tev_sql->mode_chart == Histogram){
//        plot_PRPD->hide();
//        plot_Barchart->hide();
//        plot_Histogram->show();
        plot_Histogram->replot();
//    }
}

#if 0
void TEVWidget::fresh_PRPD()
{
    int x,y;

    if( group_num != data_prpd->groupNum ){
        group_num = data_prpd->groupNum;

//        qDebug()<<"groupNum ="<<groupNum << "length =" <<data_prpd->totol;
        //处理数据
        points_origin.clear();
        QVector<QPointF> PRPS_point_list;

        for(quint32 i=0;i<data_prpd->totol;i++){

            x = (int)data_prpd->data[2*i+2];    /* FPGA要求数据对齐 */
            y = (int)data_prpd->data[2*i+3];    /* FPGA要求数据对齐 */

            transData(x,y);

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

            PRPS_point_list.append(QPointF(x,y));

//            qDebug()<<"x0="<<data_prpd->data[2*i  ] <<"\ty0="<<data_prpd->data[2*i+1];
//                qDebug()<<"x1="<<x <<"\ty1="<<y;


        }

        d_PRPD->setSamples(prpd_samples);
        plot_PRPD->replot();

        scene->addPRPD(PRPS_point_list);
    }

}
#endif

void TEVWidget::fresh_Histogram()
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

void TEVWidget::fresh_1000ms()
{
    fresh_plot();
    fresh_Histogram();
    qDebug()<<"token num:"<<token;

    d_BarChart->fresh();
}

void TEVWidget::fresh_200ms()
{
    QVector<QPointF> PRPS_point_list;

//    qDebug()<<"pulse_200ms.length()"<<pulse_200ms;
    int x,y;
    for(int i=0; i<pulse_200ms.count(); i++){
        x = pulse_200ms.at(i).x();
        y = pulse_200ms.at(i).y();
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

        PRPS_point_list.append(QPointF(x,y));
    }

    d_PRPD->setSamples(prpd_samples);
    plot_PRPD->replot();

    scene->addPRPD(PRPS_point_list);

    //刷新200ms内的db和脉冲数
//    foreach (QPoint p, pulse_200ms) {
//        if(db < qAbs(p.y()) ){
//            db = qAbs(p.y());
//        }
//    }
    pulse_number += pulse_200ms.length();

    pulse_200ms.clear();

}

//每1ms,读取一次脉冲数据
//每一次脉冲数据,生成一个脉冲点
void TEVWidget::fresh_1ms()
{
//    qDebug()<<"get new data!  "<<short_data->empty<<"\t"<<short_data->time;
    if( group_num != short_data->time ){         //判断数据有效性
        group_num = short_data->time;
        if(short_data->empty == 0){              //0为有数据
            if(token == 0){
//                qDebug()<<"token limited!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
                return;
            }
            else{
                token--;
            }
            //拷贝数据
            QVector<double> list;
            for (int i = 0; i < 256; ++i) {
                if(short_data->data[i] == 0x55aa){
                    break;
                }
                else{
                    list.append(((double)short_data->data[i] - 0x8000));
                }
            }
            //分析数据
//            qDebug()<<list;
            double max = 0, min = 0;
            foreach (double l, list) {
                max = MAX(max,l);
                min = MIN(min,l);
            }
            if(qAbs(max) > qAbs(min)){
                pulse_200ms.append(transData(short_data->time,max));
            }
            else{
                pulse_200ms.append(transData(short_data->time,min));
            }
        }
    }
}

void TEVWidget::add_token()
{
    if(token < TOKEN_MAX){
        token += 1;
    }
}

QPoint TEVWidget::transData(int x, int y)
{
    y = tev_sql->gain * TEV_FACTOR * (y - tev_sql->fpga_zero) ; //注意，脉冲计算里，忽略了噪声偏置的影响

    //取DB值
    if(y>=1){
        y = ((double)20) * log10(y);
    }
    else if(y<=-1){
        y = -((double)20) * log10(-y);
    }
    else{
        y = 0;
    }

    if(sqlcfg->get_para()->freq_val == 50){
        x = x % 2000000;    //取余数
        x = x * 360 /2000000;
    }
    else if(sqlcfg->get_para()->freq_val == 60){
        x = x % 1666667;
        x = x * 360 /1666667;
    }

    return QPoint(x,y);
}

void TEVWidget::PRPDReset()
{
    for(int i=0;i<360;i++){
        for(int j=0;j<121;j++){
            map[i][j]=0;
        }
    }
    emit tev_PRPD_data(prpd_samples);
    prpd_samples.clear();
//    fresh_PRPD();
}

void TEVWidget::maxReset()
{
    max_db = 0;
    ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "dB");
}

void TEVWidget::fresh_setting()
{
    if (tev_sql->mode == single) {
        timer_1000ms->setSingleShot(true);
        ui->comboBox->setItemText(0,tr("检测模式\t[单次]"));
    } else {
        timer_1000ms->setSingleShot(false);
        ui->comboBox->setItemText(0,tr("检测模式\t[连续]"));
    }

    if (tev_sql->mode_chart == PRPD) {
        ui->comboBox->setItemText(1,tr("图形显示\t[PRPD]"));
        plot_PRPD->show();
        plot_Barchart->hide();
        plot_Histogram->hide();
        plot_PRPS->hide();
    } else if(tev_sql->mode_chart == BASIC){
        ui->comboBox->setItemText(1,tr("图形显示 \t[时序图]"));
        plot_PRPD->hide();
        plot_Barchart->show();
        plot_Histogram->hide();
        plot_PRPS->hide();
    } else if(tev_sql->mode_chart == Histogram){
        ui->comboBox->setItemText(1,tr("图形显示 \t[柱状图]"));
        plot_PRPD->hide();
        plot_Barchart->hide();
        plot_Histogram->show();
        plot_PRPS->hide();
    } else if(tev_sql->mode_chart == PRPS){
        ui->comboBox->setItemText(1,tr("图形显示 \t[PRPS]"));
        plot_PRPD->hide();
        plot_Barchart->hide();
        plot_Histogram->hide();
        plot_PRPS->show();
    }
    ui->comboBox->setItemText(2,tr("增益调节\t[×%1]").arg(QString::number(tev_sql->gain, 'f', 1)));
    ui->comboBox->setItemText(3,tr("黄色报警阈值\t[%1]dB").arg(QString::number(tev_sql->low)));
    ui->comboBox->setItemText(4,tr("红色报警阈值\t[%1]dB").arg(QString::number(tev_sql->high)));
    if(tev_sql->auto_rec == true){
        ui->comboBox->setItemText(5,tr("自动录波\t[开启]") );
    }
    else{
        ui->comboBox->setItemText(5,tr("自动录波\t[关闭]") );
    }

    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == menu_index) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    emit fresh_parent();
}

