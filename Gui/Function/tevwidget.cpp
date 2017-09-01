#include "tevwidget.h"
#include "ui_tevwidget.h"
#include <QTimer>
#include <QLineEdit>
#include <QThreadPool>
#include "IO/Other/filetools.h"

#define VALUE_MAX  60           //RPPD最大值
#define SETTING_NUM 8           //设置菜单条目数


TEVWidget::TEVWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TevWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("TEVWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");

    key_val = val;
    this->mode = mode;
    this->menu_index = menu_index;

    reloadSql();

    this->data = data;
    db = 0;
    max_db = 0;

    pulse_cnt_last = 0;

    Common::set_comboBox_style(ui->comboBox);

    //每隔1秒，刷新
    timer1 = new QTimer(this);
    timer1->setInterval(1000);
    if (tev_sql->mode == continuous) {
        timer1->start();
    }
    connect(timer1, SIGNAL(timeout()), this, SLOT(fresh_plot()));
    connect(timer1, SIGNAL(timeout()), this, SLOT(fresh_Histogram()));

    timer2 = new QTimer(this);
    timer2->setInterval(45);
    connect(timer2, SIGNAL(timeout()), this, SLOT(fresh_PRPD()));
    timer2->start();

    PRPS_inti();
    PRPD_inti();
    histogram_init();


    recWaveForm = new RecWaveForm(menu_index,this);
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));

    logtools = new LogTools(mode);      //日志保存模块
    connect(this,SIGNAL(tev_log_data(double,int,double)),logtools,SLOT(dealLog(double,int,double)));
    connect(this,SIGNAL(tev_PRPD_data(QVector<QwtPoint3D>)),logtools,SLOT(dealRPRDLog(QVector<QwtPoint3D>)));
}

TEVWidget::~TEVWidget()
{
    delete ui;
}

void TEVWidget::showWaveData(VectorList buf, MODE mod)
{
    qDebug()<<"AAAA";
    if( (key_val->grade.val0 == menu_index)){
        key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
        key_val->grade.val5 = 1;
        recWaveForm->working(key_val,buf,mod);
        FileTools *filetools = new FileTools(buf,mod);      //开一个线程，为了不影响数据接口性能
        QThreadPool::globalInstance()->start(filetools);
    }
}

void TEVWidget::PRPS_inti()
{
    plot_PRPS = new QwtPlot(ui->widget);
    Common::set_barchart_style(plot_PRPS);
    plot_PRPS->resize(200, 140);

    d_PRPS = new BarChart(plot_PRPS, &db, &tev_sql->high, &tev_sql->low);
    connect(timer1, &QTimer::timeout, d_PRPS, &BarChart::fresh);
}

void TEVWidget::PRPD_inti()
{
    if(mode == TEV1){
        data_prpd = &data->recv_para_prpd1;
    }
    else if(mode == TEV2){
        data_prpd = &data->recv_para_prpd2;
    }

    plot_PRPD = new QwtPlot(ui->widget);
    plot_PRPD->resize(200, 140);
    d_PRPD = new QwtPlotSpectroCurve;
    Common::set_PRPD_style(plot_PRPD,d_PRPD,VALUE_MAX);

    PRPDReset();
}

void TEVWidget::histogram_init()
{
    plot_Histogram = new QwtPlot(ui->widget);
    plot_Histogram->resize(200, 150);
    d_histogram = new QwtPlotHistogram;
    Common::set_histogram_style(plot_Histogram,d_histogram);
}

//void TEVWidget::working(CURRENT_KEY_VALUE *val)
//{
//    if (val == NULL) {
//        return;
//    }
//    key_val = val;
//    reloadSql();        //重置默认数据
//    this->show();
//}

void TEVWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if(key_val->grade.val0 != menu_index){
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
        timer1->start();                                                         //and timer no stop
        if(key_val->grade.val2 == 6){
            rec_wave();     //开始录波
        }
        else if(key_val->grade.val2 == 7){
            maxReset();
        }
        else if(key_val->grade.val2 == 8){
            PRPDReset();
        }
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_CANCEL:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        reloadSql();        //重置默认数据
        timer1->start();    //重启设置后，手动开一次
//        emit
        break;
    case KEY_UP:
        if (key_val->grade.val2 < 2) {
            key_val->grade.val2 = SETTING_NUM;
        } else {
            key_val->grade.val2--;
        }
        break;
    case KEY_DOWN:
        if (key_val->grade.val2 >= SETTING_NUM) {
            key_val->grade.val2 = 1;
        } else {
            key_val->grade.val2++;
        }
        break;
    case KEY_LEFT:
        switch (key_val->grade.val2) {
        case 1:
            if (tev_sql->mode == single) {
                tev_sql->mode = continuous;
            } else {
                tev_sql->mode = single;
            }
            break;
        case 2:
            if (tev_sql->mode_chart == BASIC) {
                tev_sql->mode_chart = Histogram;
                break;
            } else if(tev_sql->mode_chart == Histogram){
                tev_sql->mode_chart = PRPD;
                break;
            } else if(tev_sql->mode_chart == PRPD){
                tev_sql->mode_chart = BASIC;
                break;
            }
        case 3:
            if (tev_sql->gain > 0.15) {
                tev_sql->gain -= 0.1;
            }
            break;
        case 4:
            if (tev_sql->low > 0) {
                tev_sql->low--;
            }
            break;
        case 5:
            if (tev_sql->high > tev_sql->low) {
                tev_sql->high--;
            }
            break;
        default:
            break;
        }
        break;

    case KEY_RIGHT:
        switch (key_val->grade.val2) {
        case 1:
            if (tev_sql->mode == single) {
                tev_sql->mode = continuous;
            } else {
                tev_sql->mode = single;
            }
            break;
        case 2:
            if (tev_sql->mode_chart == Histogram) {
                tev_sql->mode_chart = BASIC;
                break;
            } else if (tev_sql->mode_chart == BASIC) {
                tev_sql->mode_chart = PRPD;
                break;
            } else if(tev_sql->mode_chart == PRPD){
                tev_sql->mode_chart = Histogram;
                break;
            }
        case 3:
            if (tev_sql->gain < 9.95) {
                tev_sql->gain += 0.1;
            }
            break;
        case 4:
            if (tev_sql->low < tev_sql->high) {
                tev_sql->low++;
            }
            break;
        case 5:
            if (tev_sql->high < 60) {
                tev_sql->high++;
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    fresh_setting();
}

void TEVWidget::calc_tev_value (double * tev_val, double * tev_db, int * sug_central_offset, int * sug_offset)
{
    int d_max, d_min, a, b;
    double db;

    if (mode == TEV1) {
        d_max = data->recv_para_nomal.hdata0.ad.ad_max;
        d_min = data->recv_para_nomal.hdata0.ad.ad_min;
    }
    else{
        d_max = data->recv_para_nomal.hdata1.ad.ad_max;
        d_min = data->recv_para_nomal.hdata1.ad.ad_min;
    }

    * sug_central_offset = ((d_max + d_min) >> 1) - 0x8000;
    //	* sug_offset = ((d_max - d_min) >> 1) / 10;

    a = d_max - 0x8000 - tev_sql->fpga_zero;        //减去中心偏置
    b = d_min - 0x8000 - tev_sql->fpga_zero;        //减去中心偏置

    * sug_offset = ( MAX (qAbs (a), qAbs (b)) - 1 / TEV_FACTOR / tev_sql->gain ) /10;

    db = tev_sql->gain * (MAX (qAbs (a), qAbs (b)) - tev_sql->tev_offset1 * 10) * TEV_FACTOR;
    * tev_val = db;

    db = 20 * log10 (db);      //对数运算，来自工具链的函数

    if(db < 0){
        db = 0;
    }
    * tev_db = db;

    //    if( db > 50){
    //        qDebug()<<"tev_db = "<<db << "\td_max = "<<a<<"\td_min = "<<b;
    //    }
}

void TEVWidget::reloadSql()
{
    /* get sql para */
    sql_para = *sqlcfg->get_para();

    if(mode == TEV1){
        tev_sql = &sql_para.tev1_sql;
    }
    else{
        tev_sql = &sql_para.tev2_sql;
    }
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

    emit offset_suggest (sug_central_offset, sug_offset);

    //记录并显示最大值
    if (max_db < s) {
        max_db = s;
        ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "dB");
    }

    //脉冲计数和严重度
    //    signal_pulse_cnt = data->recv_para.pulse1.edge.neg + data->recv_para.pulse1.edge.pos;
    if(mode == TEV1){
        pulse_cnt = data->recv_para_nomal.hpulse0_totol;
    }
    else{
        pulse_cnt = data->recv_para_nomal.hpulse1_totol;
    }

    pulse_cnt_show = pulse_cnt_last + pulse_cnt;    //显示2秒的脉冲计数
    pulse_cnt_last = pulse_cnt;

    degree = pow(t,tev_sql->gain) * (double)pulse_cnt / sql_para.freq_val;     //严重度算法更改

    ui->label_pluse->setText(tr("脉冲数: ") + QString::number(pulse_cnt_show));
    ui->label_degree->setText(tr("严重度: ") + QString::number(degree, 'f', 2));

    ui->label_val->setText(QString::number((qint16)s));

    db = (int)s;

    emit tev_modbus_data(db,pulse_cnt_show);
    emit tev_log_data(s,pulse_cnt_show,degree);


    yc_data_type temp_data;
    if(mode == TEV1){
        temp_data.f_val = db;
        yc_set_value(TEV1_amplitude, &temp_data, 0, NULL,0);
        temp_data.f_val = pulse_cnt_show;
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
    } else if (db >= tev_sql->low) {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
    } else {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
    }

    if (tev_sql->mode_chart == PRPD) {
        plot_PRPD->show();
        plot_PRPS->hide();
        plot_Histogram->hide();
        plot_PRPD->replot();
    } else if(tev_sql->mode_chart == BASIC){
        plot_PRPD->hide();
        plot_PRPS->show();
        plot_Histogram->hide();
        plot_PRPS->replot();
    } else if(tev_sql->mode_chart == Histogram){
        plot_PRPD->hide();
        plot_PRPS->hide();
        plot_Histogram->show();
        plot_Histogram->replot();
    }
}

void TEVWidget::fresh_PRPD()
{
    int x,y;

    if( groupNum != data_prpd->groupNum ){
        groupNum = data_prpd->groupNum;
        //处理数据
        points_origin.clear();

        for(quint32 i=0;i<data_prpd->totol;i++){
            x = (int)data_prpd->data[2*i  ];
            y = (int)data_prpd->data[2*i+1];

            transData(x,y);
        }

        d_PRPD->setSamples(points);
        plot_PRPD->replot();
        emit origin_pluse_points(points_origin, groupNum);
    }

}

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

void TEVWidget::transData(int x, int y)
{
    //    y = y - 0x8000;
    //    if(y>0){
    //        y = y - tev_sql->tev_offset1;
    //    }
    //    else{
    //        y = y - tev_sql->tev_offset2;
    //    }

    y = tev_sql->gain * TEV_FACTOR * (y - 0x8000 - tev_sql->fpga_zero) ; //注意，脉冲计算里，忽略了噪声偏置的影响

    //    y = (int)(((double)y * 1000) / 32768);

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

    points_origin.append(QPoint(x,y));

    if(sqlcfg->get_para()->freq_val == 50){
        x = x *360 /2000000;
    }
    else if(sqlcfg->get_para()->freq_val == 60){
        x = x *360 /1666667;
    }

    //    x = x * 360 * 50 / 1e8;
    //    if(x>360){
    //        qDebug()<<"x="<<x;
    //    }


    if(x<360 && x>=0 && y<=60 &&y>=-60){
        QwtPoint3D p0(x,y,map[x][y+60]);
        map[x][y+60]++;
        QwtPoint3D p1(x,y,map[x][y+60]);
        if(map[x][y+60]>1){
            int n = points.indexOf(p0);
            points[n] = p1;
        }
        else{
            points.append(p1);
        }
    }
}

void TEVWidget::PRPDReset()
{
    for(int i=0;i<360;i++){
        for(int j=0;j<121;j++){
            map[i][j]=0;
        }
    }
    emit tev_PRPD_data(points);
    points.clear();
    fresh_PRPD();
}

void TEVWidget::rec_wave()
{
    emit startRecWave(mode,0);
}

void TEVWidget::maxReset()
{
    max_db = 0;
    ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "dB");
}

void TEVWidget::fresh_setting()
{
    if (tev_sql->mode == single) {
        timer1->setSingleShot(true);
        ui->comboBox->setItemText(0,tr("检测模式    [单次]"));
    } else {
        timer1->setSingleShot(false);
        ui->comboBox->setItemText(0,tr("检测模式    [连续]"));
    }
    if (tev_sql->mode_chart == PRPD) {
        ui->comboBox->setItemText(1,tr("图形显示    [PRPD]"));
        plot_PRPD->show();
        plot_PRPS->hide();
        plot_Histogram->hide();
    } else if(tev_sql->mode_chart == BASIC){
        ui->comboBox->setItemText(1,tr("图形显示  [时序图]"));
        plot_PRPD->hide();
        plot_PRPS->show();
        plot_Histogram->hide();
    } else if(tev_sql->mode_chart == Histogram){
        ui->comboBox->setItemText(1,tr("图形显示  [柱状图]"));
        plot_PRPD->hide();
        plot_PRPS->hide();
        plot_Histogram->show();
    }
    ui->comboBox->setItemText(2,tr("增益调节    [×%1]").arg(QString::number(tev_sql->gain, 'f', 1)));
    ui->comboBox->setItemText(3,tr("黄色报警阈值[%1]dB").arg(QString::number(tev_sql->low)));
    ui->comboBox->setItemText(4,tr("红色报警阈值[%1]dB").arg(QString::number(tev_sql->high)));


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

