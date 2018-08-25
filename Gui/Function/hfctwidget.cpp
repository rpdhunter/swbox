#include "hfctwidget.h"
#include "ui_hfctwidget.h"
#include <QLineEdit>
#include <QGraphicsView>
#include "../Algorithm/Wavelet/wavelet.h"

#define VALUE_MAX       9999           //RPPD最大值
#define SETTING_NUM     11           //设置菜单条目数


HFCTWidget::HFCTWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    ChannelWidget(data, val, mode, menu_index, parent),
    ui(new Ui::HFCTWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("HFCTWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    Common::set_comboBox_style(ui->comboBox);

    reload(-1);
    max_100ms = 0;

//    DWT<float> discreteWT("db4");

#ifdef TEST_LAB
    bpcable = new BpCable;
    bpcable->test();
#endif

    chart_ini();
    recWaveForm->raise();
    fir = new Fir;
    reload(menu_index);
}

HFCTWidget::~HFCTWidget()
{
    delete ui;
}

void HFCTWidget::reload(int index)
{
    //基本sql内容的初始化
    sql_para = *sqlcfg->get_para();     //重置SQL
    if(mode == HFCT1){
        short_data = &data->recv_para_short1;
        hfct_sql = &sql_para.hfct1_sql;
        mode_continuous = HFCT1_CONTINUOUS;
    }
    else if(mode == HFCT2){
        short_data = &data->recv_para_short2;
        hfct_sql = &sql_para.hfct2_sql;
        mode_continuous = HFCT2_CONTINUOUS;
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
        if(!timer_100ms->isActive()){
            timer_100ms->start();
        }
        //设置自动录波
        if( hfct_sql->auto_rec == true ){
            data->set_send_para(sp_auto_rec, menu_index + 1);
        }
        else{
            data->set_send_para(sp_auto_rec, 0);
        }
        if(mode == HFCT1){
            data->set_send_para(sp_h1_threshold, hfct_sql->fpga_threshold);
        }
        else if(mode == HFCT2){
            data->set_send_para(sp_h2_threshold, hfct_sql->fpga_threshold);
        }
        //设置滤波器
        //滤波模式设定，高八位控制H2,低八位控制H1,0无滤波，2为1.8M高通，1为500K高通
//        data->set_send_para (sp_filter_mode, sqlcfg->get_para()->hfct1_sql.filter_hp + sqlcfg->get_para()->hfct2_sql.filter_hp * 0x100);
        fresh_setting();
    }
}

void HFCTWidget::do_key_ok()
{
    sqlcfg->sql_save(&sql_para);        //保存SQL
    reload(menu_index);                 //重置默认数据
    switch (key_val->grade.val2) {
    case 9:
        emit startRecWave(mode_continuous,hfct_sql->rec_time);     //开始连续录波
        emit show_indicator(true);
//        isBusy = true;
        return;
    case 10:
        emit startRecWave(mode,0);     //开始录波
        manual = true;
        break;
    case 11:
        maxReset(ui->label_max);
        PRPDReset();
        break;
    default:
        break;
    }

//    QVector<int> list1 = Compute::sim_sin(2500,1,120);
//    QVector<int> list2 = Compute::sim_pulse(1000,120);

//    for (int i = 0; i < 120; ++i) {
//        list1[i] += list1.at(i) + list2.at(i);
//    }

//    list1 = add_filters(list1);


    ChannelWidget::do_key_ok();
}

void HFCTWidget::do_key_up_down(int d)
{
    key_val->grade.val1 = 1;
    Common::change_index(key_val->grade.val2,d,SETTING_NUM,1);
}

void HFCTWidget::do_key_left_right(int d)
{
    QList<int> list;
    switch (key_val->grade.val2) {
    case 1:
        hfct_sql->mode = !hfct_sql->mode;
        break;
    case 2:
        list << BASIC << PRPD << PRPS << TF << Spectra;
        Common::change_index(hfct_sql->chart,d,list);
        break;
    case 3:
        Common::change_index(hfct_sql->gain, d * 0.1, 20, 0.1 );
        break;
    case 4:
        list.clear();
        list << NONE << hp_500k << hp_1M << hp_1M5 << hp_1M8 << hp_2M << hp_2M5 << hp_3M << hp_5M
             << hp_8M << hp_10M << hp_12M << hp_15M << hp_18M << hp_20M << hp_22M << hp_25M << hp_28M
             << hp_30M << hp_32M << hp_35M;
        Common::adjust_filter_list(list, 0, Common::filter_to_number(hfct_sql->filter_lp));     //调整可选频率列表,使得高通截止频率永远低于低通
        Common::change_index(hfct_sql->filter_hp, d, list);
        break;
    case 5:
        list.clear();
        list << NONE << lp_2M << lp_5M << lp_8M << lp_10M << lp_12M << lp_15M << lp_18M << lp_20M
             << lp_22M << lp_25M << lp_28M << lp_30M << lp_32M << lp_35M << lp_38M << lp_40M;
        Common::adjust_filter_list(list, Common::filter_to_number(hfct_sql->filter_hp), 100);
        Common::change_index(hfct_sql->filter_lp, d, list);
        break;
    case 6:
        Common::change_index(hfct_sql->fpga_threshold, d * Common::code_value(1,mode), Common::code_value(100,mode), Common::code_value(2,mode) );
//        hfct_sql->fpga_threshold += Common::code_value(1,mode) * d;
        break;
    case 7:
        Common::change_index(hfct_sql->pulse_time, d, MAX_PULSE_CNT, 1 );
        break;
    case 8:
        hfct_sql->auto_rec = !hfct_sql->auto_rec;
        break;
    case 9:
        Common::change_index(hfct_sql->rec_time, d, 5, 1 );
        break;
    default:
        break;
    }
}

void HFCTWidget::chart_ini()
{
    //棒状图
    plot_Barchart = new QwtPlot(ui->widget);
    plot_Barchart->resize(200, 140);
    Common::set_barchart_style(plot_Barchart,VALUE_MAX);
    d_BarChart = new BarChart(plot_Barchart, &db, &hfct_sql->high, &hfct_sql->low);
    connect(timer_1000ms, SIGNAL(timeout()), d_BarChart, SLOT(fresh()) );

    //PRPS
    scene = new PRPSScene(mode,VALUE_MAX);
    plot_PRPS = new QGraphicsView(ui->widget);
    plot_PRPS->resize(ui->widget->size());
    plot_PRPS->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    plot_PRPS->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    plot_PRPS->setStyleSheet("background:transparent;color:gray;");
    plot_PRPS->setScene(scene);

    //PRPD
    plot_PRPD = new QwtPlot(ui->widget);
    plot_PRPD->resize(200, 140);
    d_PRPD = new QwtPlotSpectroCurve;
    Common::set_PRPD_style(plot_PRPD,d_PRPD,VALUE_MAX);


    //T-F
    plot_TF = new QwtPlot(ui->widget);
    plot_TF->resize(200, 140);
    d_TF = new QwtPlotSpectroCurve;
    Common::set_TF_style(plot_TF,d_TF,VALUE_MAX);

    PRPDReset();

    //Histogram
    plot_Histogram = new QwtPlot(ui->widget);
    plot_Histogram->resize(200, 140);
    d_histogram = new QwtPlotHistogram;
    Common::set_histogram_style(plot_Histogram,d_histogram,-60,60,0,100,"");

    //Spectra
    plot_Spectra = new QwtPlot(ui->widget);
    plot_Spectra->resize(200, 140);
    d_Spectra = new QwtPlotHistogram;
    Common::set_Spectra_style(plot_Spectra,d_Spectra,0,50,0,60,"");
}

void HFCTWidget::do_Spectra_compute()
{
    if(pulse_list_100ms.count() > 64){
        QVector<qint32> fft_result = fft->fft64(pulse_list_100ms.mid(0,64));
        for (int i = 0; i < 32 ; ++i) {
            Spectra_map[i] = fft_result.at(i+1);
        }
    }
    else{
        for (int i = 0; i < 32 ; ++i) {
            Spectra_map[i] = 0;
        }
    }

    Spectra_data.clear();

    for(int i=0;i<32;i++){
        QwtInterval interval( 1.5625*(i + 0.2) , 1.5625*(i + 0.8) );
        interval.setBorderFlags( QwtInterval::ExcludeMaximum );
        Spectra_data.append( QwtIntervalSample( Spectra_map[i], interval ) );
    }
    d_Spectra->setData(new QwtIntervalSeriesData( Spectra_data ));
    plot_Spectra->replot();

    pulse_list_100ms.clear();
    max_100ms = 0;
}

void HFCTWidget::PRPDReset()
{
    map_PRPD.clear();
    emit send_PRPD_data(points_PRPD);
    points_PRPD.clear();
    map_TF.clear();
    points_TF.clear();
    //    fresh_100ms();
}

void HFCTWidget::save_channel()
{
    PRPDReset();
    ChannelWidget::save_channel();
}

//读取一次原始数据
//原始数据为256个数据
void HFCTWidget::fresh_1ms()
{
    if( group_num != short_data->time ){         //判断数据有效性
        group_num = short_data->time;
        if(short_data->empty == 0){              //0为有数据
            if(token == 0){
                return;
            }
            else{
                token--;
            }

            //拷贝数据
            QVector<int> list;
            for (int i = 0; i < 256; ++i) {
                if(short_data->data[i] == 0x55aa){
                    break;
                }
                else{
                    list.append(((int)short_data->data[i] - 0x8000));
                }
            }

            //加入滤波器
            list = fir->set_filter(list, (FILTER)hfct_sql->filter_hp);
            list = fir->set_filter(list, (FILTER)hfct_sql->filter_lp);

            //加入小波滤波器
            list = Wavelet::set_filter(list, 3);

            qint32 max = MAX(max_100ms, list.at(Common::max_at(list)) );
            if(max > max_100ms ){
                pulse_list_100ms = list;            //找到100ms中最大的一组脉冲数据进行频谱分析
                max_100ms = max;
            }

            //切割 计算 筛选
            QVector<PC_DATA> pclist_1ms = Compute::compute_pc_1ms(list.mid(10,100),short_data->time,hfct_sql->gain,hfct_sql->fpga_threshold);
            //累计数据
            pclist_100ms.append(pclist_1ms);
        }
    }
}

void HFCTWidget::fresh_100ms()
{
    MyKey key_PRPD, key_TF;
    QVector<QPointF> PRPS_point_list;
    int temp_x = 0;
    double k;   //TF计算中使用的系数

    foreach (PC_DATA point, pclist_100ms) {
        if(sql_para.freq_val == 50){            //x坐标变换
            temp_x = point.phase % 2000000;    //取余数
            key_PRPD = MyKey(temp_x * 360 / 2000000 , (int)point.pc_value );       //y做处理，为了使重复点更多，节省空间
        }
        else if(sql_para.freq_val == 60){
            temp_x = point.phase % 1666667;    //取余数
            key_PRPD = MyKey(temp_x * 360 / 1666667 , (int)point.pc_value );
        }

        PRPS_point_list.append(QPointF(key_PRPD.x, key_PRPD.y));      //为PRPS图生成一次点序列

        if( map_PRPD.contains(key_PRPD) ){
            map_PRPD[key_PRPD] = map_PRPD.value(key_PRPD) + 1;
        }
        else{
            map_PRPD.insert(key_PRPD,1);
        }

        //        qDebug()<<point.pc_value << "\t" << point.rise_time << "\t" << point.fall_time;
        if(point.rise_time == 0){
            point.rise_time = 1;
        }
        k = point.fall_time / point.rise_time;
        key_TF = MyKey(point.rise_time , k * 1000 / (point.rise_time + point.fall_time) ) ;
        //        key_TF = MyKey(point.rise_time + point.fall_time , k * 1000 / (point.rise_time + point.fall_time) ) ;

        if( map_TF.contains(key_TF) ){
            map_TF[key_TF] = map_TF.value(key_TF) + 1;
        }
        else{
            map_TF.insert(key_TF,1);
        }

        points_TF.append(QwtPoint3D(key_TF.x, key_TF.y, qAbs(point.pc_value) / 40  ) );
    }

    points_PRPD.clear();
    foreach (MyKey k, map_PRPD.keys()) {
        points_PRPD.append(QwtPoint3D(k.x, k.y, map_PRPD.value(k) ) );
    }

    d_PRPD->setSamples(points_PRPD);
    plot_PRPD->setAxisScale(QwtPlot::yLeft, -VALUE_MAX, VALUE_MAX, VALUE_MAX);
    plot_PRPD->replot();

    d_TF->setSamples(points_TF);
    plot_TF->setAxisScale(QwtPlot::yLeft, 1, 100, 100);
    plot_TF->setAxisScale(QwtPlot::xBottom, 1, 100, 100);
    plot_TF->replot();

    scene->addPRPD(PRPS_point_list);        //PRPS图刷新

    do_Spectra_compute();       //频谱图

    //插入TF图,柱状图代码

    //刷新200ms内的db和脉冲数
//    qDebug()<<pclist_100ms.count();
    foreach (PC_DATA node, pclist_100ms) {
        if(db < qAbs(node.pc_value) ){
            db = qAbs(node.pc_value);
        }
//        qDebug()<<node.pc_value;
//        pclist_1000ms.append(qAbs(node.pc_value));
    }
//    pulse_number += pclist_100ms.length();

    pclist_100ms.clear();

}

void HFCTWidget::fresh_1000ms()
{
    if(db > 9999){
        db = 9999;
    }

    quint32 pulse_cnt;
    if(mode == HFCT1){
        pulse_cnt = data->recv_para_normal.hpulse0_totol;
    }
    else{
        pulse_cnt = data->recv_para_normal.hpulse1_totol;
    }
    pulse_cnt_list.append(pulse_cnt);
    if(pulse_cnt_list.count() > MAX_PULSE_CNT){
        pulse_cnt_list.removeFirst();
    }
    int pulse_cnt_show = 0;
    for (int i = 0; i < hfct_sql->pulse_time && pulse_cnt_list.count() >= i+1; ++i) {
        pulse_cnt_show += pulse_cnt_list.at(pulse_cnt_list.count() - 1 - i);
    }

    double degree = db * pulse_cnt * 1.0 / sqlcfg->get_para()->freq_val;

    ui->label_val->setText(QString("%1").arg(db));
    ui->label_pluse->setText(tr("脉冲数: ") + Common::secton_three(pulse_cnt_show) );//按三位分节法显示脉冲计数
    ui->label_degree->setText(tr("严重度: %1").arg(degree));

#ifdef TEST_LAB
    ui->label_judge->setText(tr("模式判断: %1").arg(bpcable->cable_prpd_mode(points_PRPD)));
#else
    ui->label_judge->hide();
#endif
//    pclist_1000ms.clear();

    if ( db >= hfct_sql->high) {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
        emit beep(menu_index,2);        //蜂鸣器报警
    } else if (db >= hfct_sql->low) {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
        emit beep(menu_index,1);
    } else {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
    }

    int is_current = 0;
    if((int)key_val->grade.val0 == menu_index){
        is_current = 1;
    }

    //实时数据库
    int d_max, d_min;
    if(mode == HFCT1){
        Common::rdb_set_yc_value(HFCT1_amplitude,db,is_current);
        Common::rdb_set_yc_value(HFCT1_num,pulse_cnt,is_current);
        Common::rdb_set_yc_value(HFCT1_severity,degree,is_current);
        Common::rdb_set_yc_value(HFCT1_gain,hfct_sql->gain,is_current);
        Common::rdb_set_yc_value(HFCT1_center_biased,hfct_sql->fpga_zero,is_current);
        d_max = data->recv_para_normal.hdata0.ad.ad_max;
        d_min = data->recv_para_normal.hdata0.ad.ad_min;
        Common::rdb_set_yc_value(HFCT1_center_biased_adv, ((d_max + d_min) / 2) - 0x8000, is_current);
        Common::rdb_set_yc_value(HFCT1_noise_biased,0,is_current);
        Common::rdb_set_yc_value(HFCT1_noise_biased_adv,0,is_current);
    }
    else{
        Common::rdb_set_yc_value(HFCT2_amplitude,db,is_current);
        Common::rdb_set_yc_value(HFCT2_num,pulse_cnt,is_current);
        Common::rdb_set_yc_value(HFCT2_severity,degree,is_current);
        Common::rdb_set_yc_value(HFCT2_gain,hfct_sql->gain,is_current);
        Common::rdb_set_yc_value(HFCT2_center_biased,hfct_sql->fpga_zero,is_current);
        d_max = data->recv_para_normal.hdata1.ad.ad_max;
        d_min = data->recv_para_normal.hdata1.ad.ad_min;
        Common::rdb_set_yc_value(HFCT2_center_biased_adv, ((d_max + d_min) / 2) - 0x8000, is_current);
        Common::rdb_set_yc_value(HFCT2_noise_biased,0,is_current);
        Common::rdb_set_yc_value(HFCT2_noise_biased_adv,0,is_current);
    }

    if (max_db < db) {
        max_db = db;
        ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "pC");
    }

    emit send_log_data(db,pulse_cnt,degree,is_current);

    plot_Barchart->replot();        //这里replot()包含了db清零的操作
}

void HFCTWidget::fresh_setting()
{
    if (hfct_sql->mode == single) {
        timer_1000ms->setSingleShot(true);
        ui->comboBox->setItemText(0,tr("检测模式\t[单次]"));
    } else {
        timer_1000ms->setSingleShot(false);
        ui->comboBox->setItemText(0,tr("检测模式\t[连续]"));
    }

    plot_PRPD->hide();
    plot_Barchart->hide();
    plot_PRPS->hide();
    plot_Histogram->hide();
    plot_TF->hide();
    plot_Spectra->hide();
    if (hfct_sql->chart == PRPD) {
        ui->comboBox->setItemText(1,tr("图形显示\t[PRPD]"));
        plot_PRPD->show();
    } else if(hfct_sql->chart == BASIC){
        ui->comboBox->setItemText(1,tr("图形显示 \t[时序图]"));
        plot_Barchart->show();
    } else if(hfct_sql->chart == Histogram){
        ui->comboBox->setItemText(1,tr("图形显示 \t[柱状图]"));
        plot_Histogram->show();
    }
    else if(hfct_sql->chart == PRPS){
        ui->comboBox->setItemText(1,tr("图形显示 \t[PRPS]"));
        plot_PRPS->show();
    }
    else if(hfct_sql->chart == TF){
        ui->comboBox->setItemText(1,tr("图形显示 \t[T-F图]"));
        plot_TF->show();
    }
    else if(hfct_sql->chart == Spectra){
        ui->comboBox->setItemText(1,tr("图形显示 \t[频谱图]"));
        plot_Spectra->show();
    }

    ui->comboBox->setItemText(2,tr("增益调节\t[×%1]").arg(QString::number(hfct_sql->gain, 'f', 1)));
    ui->comboBox->setItemText(3,tr("通带下限\t[%1]").arg(Common::filter_to_string(hfct_sql->filter_hp)));
    ui->comboBox->setItemText(4,tr("通带上限\t[%1]").arg(Common::filter_to_string(hfct_sql->filter_lp)));
    ui->comboBox->setItemText(5,tr("脉冲触发\t[%1]mV").arg(QString::number((int)Common::physical_value(hfct_sql->fpga_threshold,mode) )));
    ui->comboBox->setItemText(6,tr("脉冲计数时长\t[%1]s").arg(QString::number(hfct_sql->pulse_time)) );

    if(hfct_sql->auto_rec == true){
        ui->comboBox->setItemText(7,tr("自动录波\t[开启]") );
    }
    else{
        ui->comboBox->setItemText(7,tr("自动录波\t[关闭]") );
    }

    ui->comboBox->setItemText(8,tr("连续录波\t[%1]s").arg(QString::number(hfct_sql->rec_time)));

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




