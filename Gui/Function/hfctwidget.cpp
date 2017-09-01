#include "hfctwidget.h"
#include "ui_hfctwidget.h"
#include <QLineEdit>
#include <QTimer>
#include <QThreadPool>
#include "IO/Other/filetools.h"
#include "IO/rdb/rdb.h"

#define VALUE_MAX  3000           //RPPD最大值
#define SETTING_NUM 8           //设置菜单条目数


HFCTWidget::HFCTWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::HFCTWidget)
{
    ui->setupUi(this);


    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("HFCTWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");


    key_val = val;
    this->data = data;
    this->menu_index = menu_index;
    sql_para = *sqlcfg->get_para();
    pulse_number = 0;
    max_db = 0;

    Common::set_comboBox_style(ui->comboBox);

    //每隔1秒，刷新一次主界面
    timer1 = new QTimer(this);
    timer1->setInterval(1000);
    timer1->start();
    connect(timer1, SIGNAL(timeout()), this, SLOT(fresh_plot()));

    //1ms读取一次数据
    timer2 = new QTimer(this);
    timer2->setInterval(1);
    timer2->start();
    connect(timer2, SIGNAL(timeout()), this, SLOT(doHfctData()));

    //200ms刷新一次PRPD图
    timer3 = new QTimer(this);
    timer3->setInterval(200);
    timer3->start();
    connect(timer3, SIGNAL(timeout()), this, SLOT(fresh_PRPD()));

    PRPS_inti();
    PRPD_inti();

    recWaveForm = new RecWaveForm(menu_index,this);
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));
    connect(recWaveForm,SIGNAL(fresh_parent()),this,SIGNAL(fresh_parent()));

    logtools = new LogTools(mode);      //日志保存模块
    connect(this,SIGNAL(hfct_log_data(double,int,double)),logtools,SLOT(dealLog(double,int,double)));
    connect(this,SIGNAL(hfct_PRPD_data(QVector<QwtPoint3D>)),logtools,SLOT(dealRPRDLog(QVector<QwtPoint3D>)));

    //20ms发送一次短录波信号
    timer_rec_interval = new QTimer(this);
    timer_rec_interval->setSingleShot(true);
    timer_rec_interval->setInterval(1);
    connect(timer_rec_interval, SIGNAL(timeout()), this, SLOT(rec_wave()));

    timer_rec = new QTimer(this);
    timer_rec->setSingleShot(true);
    connect(timer_rec, SIGNAL(timeout()), this, SLOT(rec_wave_continuous_complete()));

    this->mode = mode;    //当前模式为HFCT
    if(mode == HFCT1){
        hfct_data = &data->recv_para_hfct1;
    }
    else if(mode == HFCT2){
        hfct_data = &data->recv_para_hfct2;
    }

    plot_PRPD->hide();
//    fresh_setting();
}

HFCTWidget::~HFCTWidget()
{
    delete ui;
}

void HFCTWidget::showWaveData(VectorList buf, MODE mod)
{
    if( key_val->grade.val0 == menu_index){
        if(mode == HFCT_CONTINUOUS){
            hfct_continuous_buf.append(buf);
            if(timer_rec->isActive()){
                timer_rec_interval->start();
            }
        }
        else if(mode == Disable){
            mode = HFCT1;
        }
        else if(mode == HFCT1){
            key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
            key_val->grade.val5 = 1;
            recWaveForm->working(key_val,buf,mod);
            FileTools *filetools = new FileTools(buf,HFCT1);      //开一个线程，为了不影响数据接口性能
            QThreadPool::globalInstance()->start(filetools);
        }
    }
}

void HFCTWidget::PRPS_inti()
{
    plot_PRPS = new QwtPlot(ui->widget);
    plot_PRPS->resize(200, 140);
    Common::set_barchart_style(plot_PRPS);

    int low = VALUE_MAX / 3;
    int high = low * 2;
    d_PRPS = new BarChart(plot_PRPS, &db, &high, &low);
    connect(timer1, SIGNAL(timeout()), d_PRPS, SLOT(fresh()) );
}

void HFCTWidget::PRPD_inti()
{
    plot_PRPD = new QwtPlot(ui->widget);
    plot_PRPD->resize(200, 140);
    d_PRPD = new QwtPlotSpectroCurve;
    Common::set_PRPD_style(plot_PRPD,d_PRPD,VALUE_MAX);
    PRPDReset();
}

void HFCTWidget::maxReset()
{
    max_db = 0;
    ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "dB");
}

void HFCTWidget::PRPDReset()
{
    map.clear();
    emit hfct_PRPD_data(points);
    points.clear();
    fresh_PRPD();
}

//发送录波信号
//第一个参数是通道号，这里固定为1通道
//第二个参数是时间，高频通道默认为0
void HFCTWidget::rec_wave()
{
    emit startRecWave(HFCT1,0);
}

void HFCTWidget::rec_wave_continuous()
{
    qDebug()<<"continuous rec begin! time = "<< sql_para.hfct1_sql.time;
    hfct_continuous_buf.clear();
    mode = HFCT_CONTINUOUS;     //标志位存放在本地

    rec_wave();

    timer_rec->setInterval(sql_para.hfct1_sql.time * 1000);
    timer_rec->start();
    timer_rec_interval->start();    //开启定时发送信号
    key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
    key_val->grade.val5 = 1;

}


void HFCTWidget::rec_wave_continuous_complete()
{
    timer_rec_interval->stop();    //关闭定时发送信号
    qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!continuous rec complete! points :"<<hfct_continuous_buf.length();
    key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
    key_val->grade.val5 = 1;
    recWaveForm->working(key_val,hfct_continuous_buf,HFCT_CONTINUOUS);  //显示数据
    FileTools *filetools = new FileTools(hfct_continuous_buf,HFCT_CONTINUOUS);      //开一个线程，为了不影响数据接口性能
    QThreadPool::globalInstance()->start(filetools);
    mode = Disable;        //用Disable做一个缓冲，处理可能出现的结尾多一包情况

}

void HFCTWidget::fresh_PRPD()
{

//    points_origin.clear();

//    len = data->recv_para.recData[1];
//    if(len == 0){       //无脉冲时，只读2数据

//    }
//    else{               //有脉冲时，不读底噪
//        for(int i=0;i<len;i++){
//            x = (int)data->recv_para.recData[2*i+4];
//            y = (int)data->recv_para.recData[2*i+5];

//            transData(x,y);
//        }
//    }
//    d_PRPD->setSamples(points);

//    plot_PRPD->replot();

    MyKey key;
    foreach (QPoint point, points_origin) {
        if(sql_para.freq_val == 50){            //x坐标变换
            key = MyKey(point.x() * 360 / 2000000 , (int)(point.y()/40)*40 );       //y做处理，为了使重复点更多，节省空间
        }
        else if(sql_para.freq_val == 60){
            key = MyKey(point.x() * 360 / 1666667 , (int)(point.y()/40)*40 );
        }

        if( map.contains(key) ){
            map[key] = map.value(key) + 1;
        }
        else{
            map.insert(key,1);
        }
    }

    points.clear();
    foreach (MyKey k, map.keys()) {
        points.append(QwtPoint3D(k.x, k.y, map.value(k) ) );
    }

    d_PRPD->setSamples(points);

    plot_PRPD->setAxisScale(QwtPlot::yLeft, -VALUE_MAX, VALUE_MAX, VALUE_MAX);

    points_origin.clear();
    plot_PRPD->replot();

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
double HFCTWidget::compute_pC(QVector<double> list , int x_origin)
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

void HFCTWidget::doHfctData()
{
    if( group != hfct_data->time ){      //判断数据有效性
        group = hfct_data->time;
        if(hfct_data->empty == 0){
            QVector<double> list;
            for (int i = 0; i < 256; ++i) {
                if(hfct_data->data[i] == 0x55aa){
                    break;
                }
                else{
                    list.append(((double)hfct_data->data[i] - 0x8000));
//                    list.append(data->recv_para.recData[i]);
                }
            }

            //计算一次数据的脉冲数
//            double max = 0;
//            foreach (double l, list) {
//                max = MAX(qAbs(l),max);
//            }
//            qDebug()<<"one pluse data number: "<< list.length() << "\t first = "<<list.first()<<"\tlast = "<<list.last()
//                      <<"\tmax = "<<max;

//            qDebug()<<list;
            pCList.append(compute_pC(list,hfct_data->time ) );
        }
    }
}


void HFCTWidget::fresh_plot()
{
    if(pCList.isEmpty()){
//        qDebug()<<"pC is : 0 ";
        db = 0;
    }
    else{
        double max = 0;
        foreach (double l, pCList) {
            max = MAX(qAbs(l),max);
        }
//        qDebug()<<pCList;
//        qDebug()<<"pC is : "<< max <<"\tpC number: " << pCList.count() << "\tpulse_number : "<< pulse_number;
        db = max * TEV_FACTOR * sql_para.hfct1_sql.gain;

        //临时加入一个坏值判定
        if(db > 5000){
            db = 0;
            pulse_number = 0;
        }

        pCList.clear();
    }

    double degree = db * pulse_number * 1.0 / sqlcfg->get_para()->freq_val;


    ui->label_val->setText(QString("%1").arg(db));
    ui->label_pluse->setText(tr("脉冲数: %1").arg(pulse_number));
    ui->label_degree->setText(tr("严重度: %1").arg(degree));

    yc_data_type temp_data;
    temp_data.f_val = db;
    yc_set_value(HFCT1_amplitude, &temp_data, 0, NULL,0);
    temp_data.f_val = pulse_number;
    yc_set_value(HFCT1_num, &temp_data, 0, NULL,0);
    temp_data.f_val = degree;
    yc_set_value(HFCT1_severity, &temp_data, 0, NULL,0);
    temp_data.f_val = sql_para.hfct1_sql.gain;
    yc_set_value(HFCT1_gain, &temp_data, 0, NULL,0);


    if (max_db < db) {
        max_db = db;
        ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "pC");
    }

    emit hfct_log_data(db,pulse_number,degree);

    pulse_number = 0;
    plot_PRPS->replot();
}


void HFCTWidget::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    sql_para = *sqlcfg->get_para();

    this->show();
}

void HFCTWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val0 != menu_index) {
        return;
    }
//    qDebug()<<"HFCT\tval0 = "<<key_val->grade.val0 <<"\tval1 = "<<key_val->grade.val1 <<"\tval2 = "<<key_val->grade.val2 ;

    if(key_val->grade.val5 != 0){
        emit send_key(key_code);
        return;
    }


    switch (key_code) {
    case KEY_OK:
        sqlcfg->sql_save(&sql_para);        //保存SQL
        timer1->start();                                                         //and timer no stop
        if(key_val->grade.val2 == 5){
            rec_wave_continuous();     //开始连续录波
            break;
        }
        else if(key_val->grade.val2 == 6){
            mode = HFCT1;
            emit startRecWave(HFCT1,0);     //开始录波
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
        sql_para = *sqlcfg->get_para();     //重置SQL
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;        
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
            if (sql_para.hfct1_sql.mode == single) {
                sql_para.hfct1_sql.mode = continuous;
            } else {
                sql_para.hfct1_sql.mode = single;
            }
            break;
        case 2:
            if (sql_para.hfct1_sql.mode_chart == BASIC) {
                sql_para.hfct1_sql.mode_chart = Histogram;
                break;
            } else if(sql_para.hfct1_sql.mode_chart == Histogram){
                sql_para.hfct1_sql.mode_chart = PRPD;
                break;
            } else if(sql_para.hfct1_sql.mode_chart == PRPD){
                sql_para.hfct1_sql.mode_chart = BASIC;
                break;
            }
        case 3:
            if (sql_para.hfct1_sql.gain > 0.15) {
                sql_para.hfct1_sql.gain -= 0.1;
            }
            break;
        case 4:
            if (sql_para.hfct1_sql.filter == NONE) {
                sql_para.hfct1_sql.filter = HP_1800K;
                data->set_send_para (sp_filter_mode, 2);
                break;
            } else if(sql_para.hfct1_sql.filter == HP_1800K){
                sql_para.hfct1_sql.filter = HP_500K;
                data->set_send_para (sp_filter_mode, 1);
                break;
            } else if(sql_para.hfct1_sql.filter == HP_500K){
                sql_para.hfct1_sql.filter = NONE;
                data->set_send_para (sp_filter_mode, 0);
                break;
            }
        case 5:
            if (sql_para.hfct1_sql.time > 0) {
                sql_para.hfct1_sql.time --;
            }
            break;
        default:
            break;
        }
        break;

    case KEY_RIGHT:
        switch (key_val->grade.val2) {
        case 1:
            if (sql_para.hfct1_sql.mode == single) {
                sql_para.hfct1_sql.mode = continuous;
            } else {
                sql_para.hfct1_sql.mode = single;
            }
            break;
        case 2:
            if (sql_para.hfct1_sql.mode_chart == Histogram) {
                sql_para.hfct1_sql.mode_chart = BASIC;
                break;
            } else if (sql_para.hfct1_sql.mode_chart == BASIC) {
                sql_para.hfct1_sql.mode_chart = PRPD;
                break;
            } else if(sql_para.hfct1_sql.mode_chart == PRPD){
                sql_para.hfct1_sql.mode_chart = Histogram;
                break;
            }
        case 3:
            if (sql_para.hfct1_sql.gain < 9.95) {
                sql_para.hfct1_sql.gain += 0.1;
            }
            break;
        case 4:
            if (sql_para.hfct1_sql.filter == HP_1800K) {
                sql_para.hfct1_sql.filter = NONE;
                data->set_send_para (sp_filter_mode, 0);
                break;
            }  else if(sql_para.hfct1_sql.filter == NONE){
                sql_para.hfct1_sql.filter = HP_500K;
                data->set_send_para (sp_filter_mode, 1);
                break;
            } else if(sql_para.hfct1_sql.filter == HP_500K){
                sql_para.hfct1_sql.filter = HP_1800K;
                data->set_send_para (sp_filter_mode, 2);
                break;
            }
        case 5:
            if (sql_para.hfct1_sql.time < 300) {
                sql_para.hfct1_sql.time ++;
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    emit fresh_parent();
    fresh_setting();
}

void HFCTWidget::fresh_setting()
{
    if (sql_para.hfct1_sql.mode == single) {
        timer1->setSingleShot(true);
        ui->comboBox->setItemText(0,tr("检测模式    [单次]"));
    } else {
        timer1->setSingleShot(false);
        ui->comboBox->setItemText(0,tr("检测模式    [连续]"));
    }
    if (sql_para.hfct1_sql.mode_chart == PRPD) {
        ui->comboBox->setItemText(1,tr("图形显示    [PRPD]"));
        plot_PRPD->show();
        plot_PRPS->hide();
//        plot_Histogram->hide();
    } else if(sql_para.hfct1_sql.mode_chart == BASIC){
        ui->comboBox->setItemText(1,tr("图形显示  [时序图]"));
        plot_PRPD->hide();
        plot_PRPS->show();
//        plot_Histogram->hide();
    } else if(sql_para.hfct1_sql.mode_chart == Histogram){
        ui->comboBox->setItemText(1,tr("图形显示  [柱状图]"));
        plot_PRPD->hide();
        plot_PRPS->hide();
//        plot_Histogram->show();
    }
    ui->comboBox->setItemText(2,tr("增益调节    [×%1]").arg(QString::number(sql_para.hfct1_sql.gain, 'f', 1)));

    if(sql_para.hfct1_sql.filter == NONE){
        ui->comboBox->setItemText(3,tr("滤波器   [无]"));        
    }
    else if(sql_para.hfct1_sql.filter == HP_500K){
        ui->comboBox->setItemText(3,tr("滤波器[高通500K]"));        
    }
    else if(sql_para.hfct1_sql.filter == HP_1800K){
        ui->comboBox->setItemText(3,tr("滤波器[高通1.8M]"));        
    }

    ui->comboBox->setItemText(4,tr("连续录波[%1]s").arg(QString::number(sql_para.hfct1_sql.time)));

    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == menu_index && key_val->grade.val5 == 0) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));

}


