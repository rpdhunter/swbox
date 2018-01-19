#include "hfctwidget.h"
#include "ui_hfctwidget.h"
#include <QLineEdit>
#include <QTimer>
#include "IO/Com/rdb/rdb.h"

#define VALUE_MAX       6000           //RPPD最大值
#define PC_MAX          VALUE_MAX
#define SETTING_NUM     9           //设置菜单条目数


HFCTWidget::HFCTWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::HFCTWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("HFCTWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    Common::set_comboBox_style(ui->comboBox);

    this->data = data;
    this->key_val = val;
    this->mode = mode;
    this->menu_index = menu_index;

    reload(-1);

    db = 0;
    low = PC_MAX / 3.0;
    high = low * 2;
    pulse_number = 0;
    max_db = 0;
    manual = false;
    isBusy = false;

    timer_1000ms = new QTimer(this);
    timer_1000ms->setInterval(1000);      //每隔1秒，刷新一次主界面
    connect(timer_1000ms, SIGNAL(timeout()), this, SLOT(fresh_1000ms()));

//    timer_1ms = new QTimer(this);
//    timer_1ms->setInterval(1);         //1ms读取一次数据
//    connect(timer_1ms, SIGNAL(timeout()), this, SLOT(fresh_1ms()));

    timer_200ms = new QTimer(this);
    timer_200ms->setInterval(200);       //200ms刷新一次PRPD图
    connect(timer_200ms, SIGNAL(timeout()), this, SLOT(fresh_200ms()));

    timer_freeze = new QTimer(this);            //timer_freeze设置了一个界面手动退出后的锁定期,便于操作
    timer_freeze->setInterval(FREEZE_TIME);      //5秒内不出现新录波界面
    timer_freeze->setSingleShot(true);

    chart_ini();

    recWaveForm = new RecWaveForm(menu_index,this);
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));
    connect(recWaveForm,SIGNAL(fresh_parent()),this,SIGNAL(fresh_parent()));
    connect(recWaveForm, SIGNAL(fresh_parent()), timer_freeze, SLOT(start()) );

    //日志保存模块
    logtools = new LogTools(mode);
    connect(this,SIGNAL(hfct_log_data(double,int,double)),logtools,SLOT(dealLog(double,int,double)));
    connect(this,SIGNAL(hfct_PRPD_data(QVector<QwtPoint3D>)),logtools,SLOT(dealRPRDLog(QVector<QwtPoint3D>)));

    reload(menu_index);
//    //设置自动录波
//    if( hfct_sql->auto_rec == true ){
//        data->set_send_para (sp_rec_on, 1);
//        data->set_send_para(sp_auto_rec, menu_index + 1);
//    }
//    else{
//        data->set_send_para (sp_rec_on, 0);
//        data->set_send_para(sp_auto_rec, 0);
//    }
//    //设置滤波器
//    data->set_send_para (sp_filter_mode, hfct_sql->filter);
//    fresh_setting();
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
//        if(!timer_1ms->isActive()){
//            timer_1ms->start();
//        }
        if(!timer_200ms->isActive()){
            timer_200ms->start();
        }
        //设置自动录波
        if( hfct_sql->auto_rec == true ){
            data->set_send_para(sp_auto_rec, menu_index + 1);
        }
        else{
            data->set_send_para(sp_auto_rec, 0);
        }
        //设置滤波器
        data->set_send_para (sp_filter_mode, hfct_sql->filter);
        fresh_setting();
    }
}

void HFCTWidget::trans_key(quint8 key_code)
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

    switch (key_code) {
    case KEY_OK:
        sqlcfg->sql_save(&sql_para);        //保存SQL
        reload(menu_index);                 //重置默认数据
        switch (key_val->grade.val2) {
        case 4:
            //设置滤波器
            data->set_send_para (sp_filter_mode, hfct_sql->filter);
            break;
        case 5:
            //设置自动录波
            if( hfct_sql->auto_rec == true ){
                data->set_send_para (sp_rec_on, 1);
                data->set_send_para(sp_auto_rec, menu_index + 1);
            }
            else{
                data->set_send_para (sp_rec_on, 0);
                data->set_send_para(sp_auto_rec, 0);
            }
            break;
        case 6:
            emit startRecWave(mode_continuous,hfct_sql->time);     //开始连续录波
            emit show_indicator(true);
            isBusy = true;
            return;
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
        reload(menu_index);
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
    }
    emit fresh_parent();
    fresh_setting();
}

void HFCTWidget::do_key_up_down(int d)
{
    key_val->grade.val1 = 1;
    Common::change_index(key_val->grade.val2,d,SETTING_NUM,1);
}

void HFCTWidget::do_key_left_right(int d)
{
    switch (key_val->grade.val2) {
    case 1:
        hfct_sql->mode = !hfct_sql->mode;
        break;
    case 2:
        Common::change_index(hfct_sql->mode_chart,d,TF,BASIC);
        break;
    case 3:
        Common::change_index(hfct_sql->gain, d * 0.1, 20, 0.1 );
        break;
    case 4:
        Common::change_index(hfct_sql->filter, d, HP_1800K, NONE );
        break;
    case 6:
        Common::change_index(hfct_sql->time, d, 20, 1 );
        break;
    case 5:
        hfct_sql->auto_rec = !hfct_sql->auto_rec;
        break;
    default:
        break;
    }
}

void HFCTWidget::showWaveData(VectorList buf, MODE mod)
{
    if( (key_val->grade.val0 == menu_index ) && ( !timer_freeze->isActive() || !hfct_sql->auto_rec || manual == true) ){
        isBusy = false;
        emit show_indicator(false);
        key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
        key_val->grade.val5 = 1;
        emit fresh_parent();
        ui->comboBox->hidePopup();
        manual = false;
        recWaveForm->working(key_val,buf,mod);
    }
}

void HFCTWidget::chart_ini()
{
    //棒状图
    plot_BarChart = new QwtPlot(ui->widget);
    plot_BarChart->resize(200, 140);
    Common::set_barchart_style(plot_BarChart,PC_MAX);
    d_barchart = new BarChart(plot_BarChart, &db, &high, &low);
    connect(timer_1000ms, SIGNAL(timeout()), d_barchart, SLOT(fresh()) );

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
    Common::set_histogram_style(plot_Histogram,d_histogram);

}

void HFCTWidget::maxReset()
{
    max_db = 0;
    ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "dB");
}

void HFCTWidget::PRPDReset()
{
    map_PRPD.clear();
    emit hfct_PRPD_data(points_PRPD);
    points_PRPD.clear();
    map_TF.clear();
    points_TF.clear();
    fresh_200ms();
}

void HFCTWidget::fresh_200ms()
{
    MyKey key_PRPD, key_TF;
    QVector<QPointF> PRPS_point_list;
    int temp_x = 0;
    double k;   //TF计算中使用的系数

    foreach (PC_DATA point, pclist_200ms) {
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

//    points_TF.clear();
//    foreach (MyKey k, map_TF.keys()) {
//        points_TF.append(QwtPoint3D(k.x, k.y, map_TF.value(k) ) );
//    }

    d_PRPD->setSamples(points_PRPD);
    plot_PRPD->setAxisScale(QwtPlot::yLeft, -VALUE_MAX, VALUE_MAX, VALUE_MAX);
    plot_PRPD->replot();

    d_TF->setSamples(points_TF);
    plot_TF->setAxisScale(QwtPlot::yLeft, 1, 100, 100);
    plot_TF->setAxisScale(QwtPlot::xBottom, 1, 100, 100);
    plot_TF->replot();

    scene->addPRPD(PRPS_point_list);        //PRPS图刷新

    //插入TF图,柱状图代码

    //刷新200ms内的db和脉冲数
    foreach (PC_DATA node, pclist_200ms) {
        if(db < qAbs(node.pc_value) ){
            db = qAbs(node.pc_value);
        }
    }
    pulse_number += pclist_200ms.length();

    pclist_200ms.clear();

}

//返回极值点的index数组
QVector<int> HFCTWidget::compute_pulse_number(QVector<double> list)
{
    QVector<int> index;

    if(list.length() <= 1){
        if(list.length() == 1){
            index.append(0);
        }
        return index;
    }


    for (int i = 0; i < list.length(); ++i) {
        list[i] = qAbs(list.at(i));         //取绝对值
    }

    bool max_flag = false;  //存在极值点的标志

    for (int i = 1; i < list.length(); ++i) {
        if(list.at(i-1) < 1500 && list.at(i) > 1500){
            max_flag = true;
        }
        if(max_flag == true && list.at(i-1) > list.at(i)){
            index.append(i-1);  //这就是一个极值点
            max_flag = false;
        }
    }
    //    qDebug()<<"n = "<< number;

    return index;
}

//根据读取到的序列，计算电缆局放值
double HFCTWidget::compute_list_pC(QVector<double> list , int x_origin)
{
    if(list.length() < 2 )
        return 0;

    QVector<double> pC;
    int first=0,last=0;
    double one_pC;
    for (int i = 0; i < list.length() - 1; ++i) {
        if(list.at(i) * list.at(i+1) <= 0){
            last = i+1;
            if(last-first > 1){ //至少有3个点
                one_pC = compute_one_pC(list.mid(first,last-first+1));  //计算一个脉冲的pC值
                first = i;
                if(one_pC != 0){            //认为值严格为0的点是无意义点
                    pC.append( one_pC );
                }
            }
        }
    }

    if(pC.isEmpty()){   //说明序列没有子脉冲，则把整个序列看做一个脉冲
        pC.append(compute_one_pC(list) );
    }

    //    qDebug()<<"divide:"<<pC;

    //计算原始脉冲点
    QVector<int> index = compute_pulse_number(pC);
    foreach (int in, index) {
        points_origin.append( QPoint( x_origin, pC.at(in) ));
    }

    //脉冲数
    pulse_number += index.length();

    //计算脉冲值（最大值）
    double max = 0;
    foreach (double l, pC) {
        max = MAX(qAbs(l),max);
    }
    return max;
}

double HFCTWidget::compute_one_pC(QVector<double> list)
{
    //    qDebug()<<list;
    if(list.length() < 2 )
        return 0;

    double t1=0, t2=0, s=0;
    int first = 0, last = list.length()-1;

    if(list.at(0) * list.at(1) <= 0){
        t1 = triangle(list.at(1), list.at(0));
        first = 1;
    }

    if(list.at(last-1) * list.at(last) <= 0){
        t2 = triangle(list.at(last-1), list.at(last));
        last = list.length() - 2;
    }

    s = simpson(list.mid(first,last - first + 1));

    //    qDebug()<<"t1="<<t1<<"\ts="<<s<<"\tt2="<<t2<<"\t"<<list << "simpson :"<<list.mid(first,last - first + 1);

    return t1 + s + t2 ;
}

//list是已标准化的脉冲序列（如何标准化，以后探讨）
//list长度至少为2
//函数实现用复化辛普生公式求积分
double HFCTWidget::simpson(QVector<double> list)
{
    if(list.length() < 2 )
        return 0;

    if(list.length() == 2)
        return (list.at(0) + list.at(1)) / 2 ;      //梯形公式

    int n = (list.length()-1) / 2 ;     //n至少为1
    double S = list.at(0) - list.at(2*n);
    for (int i = 1; i <= n; ++i) {
        S += 4*list.at(2*i-1) + 2*list.at(2*i);
    }
    if(list.length() % 2 == 1){     //如果长度为奇数，正好使用复化辛普生公式，否则补上一个梯形公式做结尾
        return S / 3;
    }
    else{
        return (list.at(2*n) + list.last() )/2 + S/3 ;
    }
}

//返回序列首（尾）遇到符号变化时，三角形的面积
//d1为靠近序列内部的数据点，d2为序列首（尾）部数据点
double HFCTWidget::triangle(double d1, double d2)
{
    return d1*d1 / (d1 - d2) / 2;
}

//读取一次原始数据
//原始数据为256个数据
void HFCTWidget::fresh_1ms()
{
    if( group_num != short_data->time ){         //判断数据有效性
        group_num = short_data->time;
        if(short_data->empty == 0){              //0为有数据
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
            //切割 计算 筛选
            QVector<PC_DATA> pclist_1ms = compute_pc_1ms(list,short_data->time);
            //累计数据
            pclist_200ms.append(pclist_1ms);
        }
    }
}

void HFCTWidget::fresh_1000ms()
{
    if(db > 9999){
        db = 9999;
    }


    double degree = db * pulse_number * 1.0 / sqlcfg->get_para()->freq_val;

    ui->label_val->setText(QString("%1").arg(db));
    ui->label_pluse->setText(tr("脉冲数: %1").arg(pulse_number));
    ui->label_degree->setText(tr("严重度: %1").arg(degree));

    if(db >= 4000){
        emit beep(menu_index, 2);        //蜂鸣器报警
    }
    else if( db >= 2000){
        emit beep(menu_index, 1);
    }

    yc_data_type temp_data;
    if(mode == HFCT1){
        temp_data.f_val = db;
        yc_set_value(HFCT1_amplitude, &temp_data, 0, NULL,0);
        temp_data.f_val = pulse_number;
        yc_set_value(HFCT1_num, &temp_data, 0, NULL,0);
        temp_data.f_val = degree;
        yc_set_value(HFCT1_severity, &temp_data, 0, NULL,0);
        temp_data.f_val = hfct_sql->gain;
        yc_set_value(HFCT1_gain, &temp_data, 0, NULL,0);
        temp_data.f_val = 0;
        yc_set_value(HFCT1_center_biased, &temp_data, 0, NULL,0);
        temp_data.f_val = 0;
        yc_set_value(HFCT1_center_biased_adv, &temp_data, 0, NULL,0);
        temp_data.f_val = 0;
        yc_set_value(HFCT1_noise_biased, &temp_data, 0, NULL,0);
        temp_data.f_val = 0;
        yc_set_value(HFCT1_noise_biased_adv, &temp_data, 0, NULL,0);
    }
    else{
        temp_data.f_val = db;
        yc_set_value(HFCT2_amplitude, &temp_data, 0, NULL,0);
        temp_data.f_val = pulse_number;
        yc_set_value(HFCT2_num, &temp_data, 0, NULL,0);
        temp_data.f_val = degree;
        yc_set_value(HFCT2_severity, &temp_data, 0, NULL,0);
        temp_data.f_val = hfct_sql->gain;
        yc_set_value(HFCT2_gain, &temp_data, 0, NULL,0);
        temp_data.f_val = 0;
        yc_set_value(HFCT2_center_biased, &temp_data, 0, NULL,0);
        temp_data.f_val = 0;
        yc_set_value(HFCT2_center_biased_adv, &temp_data, 0, NULL,0);
        temp_data.f_val = 0;
        yc_set_value(HFCT2_noise_biased, &temp_data, 0, NULL,0);
        temp_data.f_val = 0;
        yc_set_value(HFCT2_noise_biased_adv, &temp_data, 0, NULL,0);
    }

    if (max_db < db) {
        max_db = db;
        ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "pC");
    }

    emit hfct_log_data(db,pulse_number,degree);

    plot_BarChart->replot();        //这里replot()包含了db清零的操作
//    db = 0;
    pulse_number = 0;

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

    if (hfct_sql->mode_chart == PRPD) {
        ui->comboBox->setItemText(1,tr("图形显示\t[PRPD]"));
        plot_PRPD->show();
        plot_BarChart->hide();
        plot_PRPS->hide();
        plot_Histogram->hide();
        plot_TF->hide();
    } else if(hfct_sql->mode_chart == BASIC){
        ui->comboBox->setItemText(1,tr("图形显示 \t[时序图]"));
        plot_PRPD->hide();
        plot_BarChart->show();
        plot_PRPS->hide();
        plot_Histogram->hide();
        plot_TF->hide();
    } else if(hfct_sql->mode_chart == Histogram){
        ui->comboBox->setItemText(1,tr("图形显示 \t[柱状图]"));
        plot_PRPD->hide();
        plot_BarChart->hide();
        plot_PRPS->hide();
        plot_Histogram->show();
        plot_TF->hide();
    }
    else if(hfct_sql->mode_chart == PRPS){
        ui->comboBox->setItemText(1,tr("图形显示 \t[PRPS]"));
        plot_PRPD->hide();
        plot_BarChart->hide();
        plot_PRPS->show();
        plot_Histogram->hide();
        plot_TF->hide();
    }
    else if(hfct_sql->mode_chart == TF){
        ui->comboBox->setItemText(1,tr("图形显示 \t[T-F图]"));
        plot_PRPD->hide();
        plot_BarChart->hide();
        plot_PRPS->hide();
        plot_Histogram->hide();
        plot_TF->show();
    }

    ui->comboBox->setItemText(2,tr("增益调节\t[×%1]").arg(QString::number(hfct_sql->gain, 'f', 1)));

    if(hfct_sql->filter == NONE){
        ui->comboBox->setItemText(3,tr("滤波器\t[无]"));
    }
    else if(hfct_sql->filter == HP_500K){
        ui->comboBox->setItemText(3,tr("滤波器   [高通500K]"));
    }
    else if(hfct_sql->filter == HP_1800K){
        ui->comboBox->setItemText(3,tr("滤波器   [高通1.8M]"));
    }

    ui->comboBox->setItemText(5,tr("连续录波\t[%1]s").arg(QString::number(hfct_sql->time)));
    if(hfct_sql->auto_rec == true){
        ui->comboBox->setItemText(4,tr("自动录波\t[开启]") );
    }
    else{
        ui->comboBox->setItemText(4,tr("自动录波\t[关闭]") );
    }

    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == menu_index && key_val->grade.val5 == 0) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
}

QVector<HFCTWidget::PC_DATA> HFCTWidget::compute_pc_1ms(QVector<double> list, int x_origin)
{
    QVector<PC_DATA> pclist_1ms;

    if(list.length() < 2 )
        return pclist_1ms;

    PC_DATA pc_data;
    int first=0,last=0;
    for (int i = 0; i < list.length() - 1; ++i) {
        if(list.at(i) * list.at(i+1) <= 0){
            last = i+1;
            if(last-first > 1){         //至少有3个点
                pc_data = compute_pc_1node(list.mid(first,last-first+1), x_origin);  //计算一个脉冲的pC值
                first = i;
                if(qAbs(pc_data.pc_value) > 10){            //认为值严格为0的点是无意义点(可加入更严格的筛选条件)
                    pclist_1ms.append( pc_data );
                }
            }
        }
    }

    if(pclist_1ms.isEmpty()){           //序列没有子脉冲，则把整个序列看做一个脉冲
        pclist_1ms.append(compute_pc_1node(list, x_origin) );
    }

    return pclist_1ms;
}

HFCTWidget::PC_DATA HFCTWidget::compute_pc_1node(QVector<double> list, int x_origin)
{
    PC_DATA pc_data;
    pc_data.pc_value = 0;

    if(list.length() < 2 )
        return pc_data;

    double t1=0, t2=0, s=0;                 //前后中面积
    int first = 0, last = list.length()-1;  //前中,后中分割点
    double first_zero = first, last_zero = last, peak = first;     //第一个零点,第二个零点,峰值点

    if(list.at(0) * list.at(1) <= 0 && list.at(0) != list.at(1)){       //判断符号变化,且不同时为0
        t1 = triangle(list.at(1), list.at(0));
        first_zero = list.at(0) / (list.at(0) - list.at(1));
        first = 1;        
    }

    if(list.at(last-1) * list.at(last) <= 0 && list.at(last-1) != list.at(last)){
        t2 = triangle(list.at(last-1), list.at(last));
        last_zero = last - 1 + list.at(last-1) / (list.at(last-1) - list.at(last));
        last = list.length() - 2; 
    }

    s = simpson(list.mid(first,last - first + 1));

    //求峰值位置
    peak = first;
    for (int i = 0; i < last - first + 1; ++i) {
        if( qAbs(list.at(first + i))  > qAbs( list.at(peak)) ) {
            peak = first + i;
        }
    }

    //    qDebug()<<"t1="<<t1<<"\ts="<<s<<"\tt2="<<t2<<"\t"<<list << "simpson :"<<list.mid(first,last - first + 1);

    pc_data.pc_value = hfct_sql->gain * TEV_FACTOR * (t1 + s + t2);
    pc_data.phase = x_origin;

    pc_data.rise_time = 10 * (peak - first_zero);       //单位为ns
    pc_data.fall_time = 10 * (last_zero - peak);        //单位为ns
    return pc_data;
}




