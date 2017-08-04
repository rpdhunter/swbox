#include "rfctwidget.h"
#include "ui_rfctwidget.h"
#include <QLineEdit>
#include <QTimer>
#include <qwt_plot.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_spectrocurve.h>
#include <qwt_color_map.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_histogram.h>
#include "../Menu0/barchart.h"
#include <QThreadPool>
#include "IO/Data/filetools.h"

#define VALUE_MAX  3000           //RPPD最大值
#define SETTING_NUM 9           //设置菜单条目数


RFCTWidget::RFCTWidget(QWidget *parent, G_PARA *g_data) :
    QFrame(parent),
    ui(new Ui::RFCTWidget)
{
    ui->setupUi(this);

    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("RFCTWidget {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");

    key_val = NULL;
    data = g_data;
    sql_para = *sqlcfg->get_para();
    pulse_number = 0;
    max_db = 0;

    QLineEdit *lineEdit = new QLineEdit;
    ui->comboBox->setStyleSheet("QComboBox {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray; }");
    ui->comboBox->setLineEdit(lineEdit);
    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    ui->comboBox->lineEdit()->setReadOnly(true);
    ui->comboBox->lineEdit()->setStyleSheet("QLineEdit {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    ui->comboBox->view()->setStyleSheet("QListView {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray;outline: none;}");

//    timer = new QTimer(this);
//    timer->setSingleShot(true);
//    timer->start(3000);
//    connect(timer,SIGNAL(timeout()),this,S

    //每隔1秒，刷新一次主界面
    timer1 = new QTimer(this);
    timer1->setInterval(1000);
    timer1->start();
    connect(timer1, SIGNAL(timeout()), this, SLOT(fresh_plot()));

    //1ms读取一次数据
    timer2 = new QTimer(this);
    timer2->setInterval(1);
    timer2->start();
    connect(timer2, SIGNAL(timeout()), this, SLOT(doRfctData()));

    //200ms刷新一次PRPD图
    timer3 = new QTimer(this);
    timer3->setInterval(200);
    timer3->start();
    connect(timer3, SIGNAL(timeout()), this, SLOT(fresh_PRPD()));

    PRPS_inti();
    PRPD_inti();

    recWaveForm = new RecWaveForm(this);
    recWaveForm->hide();
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));
    connect(recWaveForm,SIGNAL(fresh_parent()),this,SIGNAL(fresh_parent()));

    logtools = new LogTools(MODE::RFCT);      //日志保存模块
    connect(this,SIGNAL(rfct_log_data(double,int,double)),logtools,SLOT(dealLog(double,int,double)));
    connect(this,SIGNAL(rfct_PRPD_data(QVector<QwtPoint3D>)),logtools,SLOT(dealRPRDLog(QVector<QwtPoint3D>)));

    //20ms发送一次短录波信号
    timer_rec_interval = new QTimer(this);
    timer_rec_interval->setSingleShot(true);
    timer_rec_interval->setInterval(1);
    connect(timer_rec_interval, SIGNAL(timeout()), this, SLOT(rec_wave()));

    timer_rec = new QTimer(this);
    timer_rec->setSingleShot(true);
    connect(timer_rec, SIGNAL(timeout()), this, SLOT(rec_wave_continuous_complete()));

    current_mode = RFCT;    //当前模式为RFCT

}

RFCTWidget::~RFCTWidget()
{
    delete ui;
}

void RFCTWidget::showWaveData(VectorList buf, MODE mod)
{
    if( key_val->grade.val0 == 5){
        if(current_mode == RFCT_CONTINUOUS){
            rfct_continuous_buf.append(buf);
            if(timer_rec->isActive()){
                timer_rec_interval->start();
            }
        }
        else if(current_mode == Disable){
            current_mode = RFCT;
        }
        else if(current_mode == RFCT){
            key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
            key_val->grade.val5 = 1;
            recWaveForm->working(key_val,buf,mod);
            FileTools *filetools = new FileTools(buf,RFCT);      //开一个线程，为了不影响数据接口性能
            QThreadPool::globalInstance()->start(filetools);
        }
    }
}

void RFCTWidget::PRPS_inti()
{
    plot_PRPS = new QwtPlot(ui->widget);
    plot_PRPS->resize(200, 140);
    plot_PRPS->setStyleSheet("background:transparent;color:gray;");

    plot_PRPS->setAxisScale(QwtPlot::xBottom, 0, 13);
    plot_PRPS->setAxisScale(QwtPlot::yLeft, 0, VALUE_MAX, VALUE_MAX / 3);

    plot_PRPS->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot_PRPS->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    //plot->setAxisMaxMajor(QwtPlot::yLeft, 2);

    plot_PRPS->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot_PRPS->axisWidget(QwtPlot::yLeft)->setMargin(0);
    plot_PRPS->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot_PRPS->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot_PRPS->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    plot_PRPS->plotLayout()->setAlignCanvasToScales(true);

    int low = VALUE_MAX / 3;
    int high = low * 2;
    d_PRPS = new BarChart(plot_PRPS, &db, &high, &low);
    connect(timer1, SIGNAL(timeout()), d_PRPS, SLOT(fresh()) );
}

void RFCTWidget::PRPD_inti()
{
    plot_PRPD = new QwtPlot(ui->widget);
    plot_PRPD->resize(200, 140);
    plot_PRPD->setStyleSheet("background:transparent;color:gray;");

    plot_PRPD->setAxisScale(QwtPlot::xBottom, 0, 360, 180);
    plot_PRPD->setAxisScale(QwtPlot::yLeft, -VALUE_MAX, VALUE_MAX, VALUE_MAX);

    plot_PRPD->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot_PRPD->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    /* remove gap */
    plot_PRPD->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot_PRPD->axisWidget(QwtPlot::yLeft)->setMargin(0);

//    plot_PRPD->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );  //对数坐标
    plot_PRPD->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot_PRPD->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot_PRPD->plotLayout()->setAlignCanvasToScales(true);

    QwtPlotCurve *curve_grid = new QwtPlotCurve();
    curve_grid->setPen(QPen(Qt::gray, 0, Qt::SolidLine, Qt::RoundCap));
    curve_grid->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve_grid->attach(plot_PRPD);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen( Qt::gray, 0.0, Qt::DotLine );
    grid->enableX( true );
    grid->enableXMin( false );
    grid->enableY( true );
    grid->enableYMin( false );
    grid->attach(plot_PRPD);

    QVector<double> X,Y;
    for(int i=0;i<360;i++){
        X.append(i);
        Y.append(VALUE_MAX*qSin(2*3.1416*i/360));
    }
    curve_grid->setSamples(X,Y);

    d_PRPD = new QwtPlotSpectroCurve;
//    d_curve->setPenWidth(2);

    QwtLinearColorMap *colorMap = new QwtLinearColorMap;
    colorMap->setColorInterval(Qt::blue,Qt::red);
    colorMap->addColorStop(0.3,Qt::green);
    colorMap->addColorStop(0.6,Qt::yellow);
    d_PRPD->setColorMap(colorMap);
    d_PRPD->setColorRange(QwtInterval(1,6));
    QwtScaleWidget *rightAxis = plot_PRPD->axisWidget( QwtPlot::yRight );
    rightAxis->setColorBarEnabled( true );
    rightAxis->setColorMap(QwtInterval(1,6),colorMap);

    plot_PRPD->setAxisScale( QwtPlot::yRight, 1, 6 );
    plot_PRPD->enableAxis( QwtPlot::yRight );


    plot_PRPD->plotLayout()->setAlignCanvasToScales( true );
    d_PRPD->attach(plot_PRPD);

    PRPDReset();
}

void RFCTWidget::maxReset()
{
    max_db = 0;
    ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "dB");
}

void RFCTWidget::PRPDReset()
{
    map.clear();
    emit rfct_PRPD_data(points);
    points.clear();
    fresh_PRPD();
}

//发送录波信号
//第一个参数是通道号，这里固定为1通道
//第二个参数是时间，高频通道默认为0
void RFCTWidget::rec_wave()
{
    emit startRecWave(RFCT,0);
}

void RFCTWidget::rec_wave_continuous()
{
    qDebug()<<"continuous rec begin! time = "<< sql_para.rfct_sql.time;
    rfct_continuous_buf.clear();
    current_mode = RFCT_CONTINUOUS;     //标志位存放在本地

    rec_wave();

    timer_rec->setInterval(sql_para.rfct_sql.time * 1000);
    timer_rec->start();
    timer_rec_interval->start();    //开启定时发送信号
    key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
    key_val->grade.val5 = 1;

}


void RFCTWidget::rec_wave_continuous_complete()
{
    timer_rec_interval->stop();    //关闭定时发送信号
    qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!continuous rec complete! points :"<<rfct_continuous_buf.length();
    key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
    key_val->grade.val5 = 1;
    recWaveForm->working(key_val,rfct_continuous_buf,RFCT_CONTINUOUS);  //显示数据
    FileTools *filetools = new FileTools(rfct_continuous_buf,RFCT_CONTINUOUS);      //开一个线程，为了不影响数据接口性能
    QThreadPool::globalInstance()->start(filetools);
    current_mode = Disable;        //用Disable做一个缓冲，处理可能出现的结尾多一包情况

}

void RFCTWidget::fresh_PRPD()
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
QVector<int> RFCTWidget::compute_pulse_number(QVector<double> list)
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
double RFCTWidget::compute_pC(QVector<double> list , int x_origin)
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

double RFCTWidget::compute_one_pC(QVector<double> list)
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
double RFCTWidget::simpson(QVector<double> list)
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
double RFCTWidget::triangle(double d1, double d2)
{
    return d1*d1 / (d1 - d2) / 2;
}

void RFCTWidget::doRfctData()
{
    if(group != data->recv_para.groupNum){      //判断数据有效性
        group = data->recv_para.groupNum;
        if(data->recv_para.rfctData[0] == 0){
            QVector<double> list;
            for (int i = 2; i < 254; ++i) {
                if(data->recv_para.rfctData[i] == 0x55aa){
                    break;
                }
                else{
                    list.append(((double)data->recv_para.rfctData[i] - 0x8000));
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
            pCList.append(compute_pC(list,data->recv_para.rfctData[1]) );
        }
    }
}


void RFCTWidget::fresh_plot()
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
        db = max * TEV_FACTOR * sql_para.rfct_sql.gain;

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


    if (max_db < db) {
        max_db = db;
        ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "pC");
    }

    emit rfct_log_data(db,pulse_number,degree);

    pulse_number = 0;
    plot_PRPS->replot();
}


void RFCTWidget::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    sql_para = *sqlcfg->get_para();

    this->show();
}

void RFCTWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val0 != 5) {
        return;
    }

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
            current_mode = RFCT;
            emit startRecWave(RFCT,0);     //开始录波
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
            if (sql_para.rfct_sql.mode == single) {
                sql_para.rfct_sql.mode = continuous;
            } else {
                sql_para.rfct_sql.mode = single;
            }
            break;
        case 2:
            if (sql_para.rfct_sql.mode_chart == PRPS) {
                sql_para.rfct_sql.mode_chart = Histogram;
                break;
            } else if(sql_para.rfct_sql.mode_chart == Histogram){
                sql_para.rfct_sql.mode_chart = PRPD;
                break;
            } else if(sql_para.rfct_sql.mode_chart == PRPD){
                sql_para.rfct_sql.mode_chart = PRPS;
                break;
            }
        case 3:
            if (sql_para.rfct_sql.gain > 0.15) {
                sql_para.rfct_sql.gain -= 0.1;
            }
            break;
        case 4:
            if (sql_para.rfct_sql.filter == NONE) {
                sql_para.rfct_sql.filter = HP_1800K;
                data->set_send_para (sp_working_mode_ad2, 2);
                break;
            } else if(sql_para.rfct_sql.filter == HP_1800K){
                sql_para.rfct_sql.filter = HP_500K;
                data->set_send_para (sp_working_mode_ad2, 1);
                break;
            } else if(sql_para.rfct_sql.filter == HP_500K){
                sql_para.rfct_sql.filter = NONE;
                data->set_send_para (sp_working_mode_ad2, 3);
                break;
            }
        case 5:
            if (sql_para.rfct_sql.time > 0) {
                sql_para.rfct_sql.time --;
            }
            break;
        default:
            break;
        }
        break;

    case KEY_RIGHT:
        switch (key_val->grade.val2) {
        case 1:
            if (sql_para.rfct_sql.mode == single) {
                sql_para.rfct_sql.mode = continuous;
            } else {
                sql_para.rfct_sql.mode = single;
            }
            break;
        case 2:
            if (sql_para.rfct_sql.mode_chart == Histogram) {
                sql_para.rfct_sql.mode_chart = PRPS;
                break;
            } else if (sql_para.rfct_sql.mode_chart == PRPS) {
                sql_para.rfct_sql.mode_chart = PRPD;
                break;
            } else if(sql_para.rfct_sql.mode_chart == PRPD){
                sql_para.rfct_sql.mode_chart = Histogram;
                break;
            }
        case 3:
            if (sql_para.rfct_sql.gain < 9.95) {
                sql_para.rfct_sql.gain += 0.1;
            }
            break;
        case 4:
            if (sql_para.rfct_sql.filter == HP_1800K) {
                sql_para.rfct_sql.filter = NONE;
                data->set_send_para (sp_working_mode_ad2, 3);
                break;
            }  else if(sql_para.rfct_sql.filter == NONE){
                sql_para.rfct_sql.filter = HP_500K;
                data->set_send_para (sp_working_mode_ad2, 1);
                break;
            } else if(sql_para.rfct_sql.filter == HP_500K){
                sql_para.rfct_sql.filter = HP_1800K;
                data->set_send_para (sp_working_mode_ad2, 2);
                break;
            }
        case 5:
            if (sql_para.rfct_sql.time < 300) {
                sql_para.rfct_sql.time ++;
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

void RFCTWidget::fresh_setting()
{
    if (sql_para.rfct_sql.mode == single) {
        timer1->setSingleShot(true);
        ui->comboBox->setItemText(0,tr("检测模式    [单次]"));
    } else {
        timer1->setSingleShot(false);
        ui->comboBox->setItemText(0,tr("检测模式    [连续]"));
    }
    if (sql_para.rfct_sql.mode_chart == PRPD) {
        ui->comboBox->setItemText(1,tr("图形显示    [PRPD]"));
        plot_PRPD->show();
        plot_PRPS->hide();
//        plot_Histogram->hide();
    } else if(sql_para.rfct_sql.mode_chart == PRPS){
        ui->comboBox->setItemText(1,tr("图形显示  [时序图]"));
        plot_PRPD->hide();
        plot_PRPS->show();
//        plot_Histogram->hide();
    } else if(sql_para.rfct_sql.mode_chart == Histogram){
        ui->comboBox->setItemText(1,tr("图形显示  [柱状图]"));
        plot_PRPD->hide();
        plot_PRPS->hide();
//        plot_Histogram->show();
    }
    ui->comboBox->setItemText(2,tr("增益调节    [×%1]").arg(QString::number(sql_para.rfct_sql.gain, 'f', 1)));
//    ui->comboBox->setItemText(3,tr("黄色报警阈值[%1]dB").arg(QString::number(sql_para.rfct_sql.low)));
//    ui->comboBox->setItemText(4,tr("红色报警阈值[%1]dB").arg(QString::number(sql_para.rfct_sql.high)));

    if(sql_para.rfct_sql.filter == NONE){
        ui->comboBox->setItemText(3,tr("滤波器   [无]"));        
    }
    else if(sql_para.rfct_sql.filter == HP_500K){
        ui->comboBox->setItemText(3,tr("滤波器[高通500K]"));        
    }
    else if(sql_para.rfct_sql.filter == HP_1800K){
        ui->comboBox->setItemText(3,tr("滤波器[高通1.8M]"));        
    }

    ui->comboBox->setItemText(4,tr("连续录波[%1]s").arg(QString::number(sql_para.rfct_sql.time)));

    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == 5 && key_val->grade.val5 == 0) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));

}


