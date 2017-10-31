#include "faultlocation.h"
#include "ui_faultlocation.h"
#include <QLineEdit>
#include <qwt_dial_needle.h>
#include <QTimer>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include "IO/Other/filetools.h"
#include <QThreadPool>
#include "IO/Com/rdb/rdb.h"

#define SHOW_FACTOR 4.0     //保存默认的也是最小的纵轴间距
#define SETTING_NUM 5           //设置菜单条目数

#define X_SCALE 2.0     //X拉伸因子

FaultLocation::FaultLocation(G_PARA *data, CURRENT_KEY_VALUE *val, QList<MODE> mode_list, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FaultLocation)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("FaultLocation {border-image: url(:/widgetphoto/bk/bk2.png);}");
    Common::set_comboBox_style(ui->comboBox);

    this->data = data;
    this->key_val = val;
    this->mode_list = mode_list;        //注意,根据初始化顺序,这里mode_list只有2个值,存储的是高速通道信息
//    qDebug()<< "mode_list" <<mode_list.count();
    this->menu_index = menu_index;

    reload(-1);

    compass_init();

    ui->label_channel1->setText( Common::MODE_toString(mode_list.at(0)));
    ui->label_channel2->setText( Common::MODE_toString(mode_list.at(1)));

    //脉冲触发计时器
    timer = new QTimer;
    connect(timer,SIGNAL(timeout()),this,SLOT(processingAScan()) );

    //进度条
    timer2 = new QTimer;
    timer2->setInterval(50);
    connect(timer2,SIGNAL(timeout()),this,SLOT(showProgress()));

    plot_init();

    reload(menu_index);
}

FaultLocation::~FaultLocation()
{
    delete ui;
}

void FaultLocation::compass_init()
{
    //指针和刻度
    QwtDialSimpleNeedle *needle = new QwtDialSimpleNeedle( QwtDialSimpleNeedle::Arrow,false, Qt::red );
    needle->setWidth(8);
    ui->Compass->setNeedle( needle );

    QwtCompassScaleDraw *scaleDraw = new QwtCompassScaleDraw();
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Ticks, true );
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Labels, true );
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
    scaleDraw->setTickLength( QwtScaleDiv::MinorTick, 0 );
    scaleDraw->setTickLength( QwtScaleDiv::MediumTick, 0 );
    scaleDraw->setTickLength( QwtScaleDiv::MajorTick, 3 );

    ui->Compass->setScaleDraw( scaleDraw );

    ui->Compass->setValue(0);

    //指针平滑移动
    from = 360;
    to = 360;
    speed = 50;
    timer1 = new QTimer;
    timer1->setInterval(2000/4/speed);
    connect(timer1,SIGNAL(timeout()),this,SLOT(setMiniCompassValue()));
    timer1->start();
}

void FaultLocation::plot_init()
{
    //绘图
    plot = new QwtPlot(this);
    /* remove gap */
    plot->axisWidget(QwtPlot::yLeft)->setTitle("V m");
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setSpacing(10);
    plot->axisWidget(QwtPlot::xBottom)->setTitle("us");
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::xBottom)->setSpacing(0);


    plot->resize(300,165);
    plot->move(70,25);
    plot->setStyleSheet("background:transparent;color:gray;");

    curve1 = new QwtPlotCurve();
    curve1->setPen(QPen(Qt::red, 0, Qt::SolidLine, Qt::RoundCap));
    curve1->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve1->attach(plot);

    curve2 = new QwtPlotCurve();
    curve2->setPen(QPen(Qt::yellow, 0, Qt::SolidLine, Qt::RoundCap));
    curve2->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve2->attach(plot);

    plot->hide();
}

void FaultLocation::reload(int index)
{
    //基本sql内容的初始化
    sql_para = *sqlcfg->get_para();
    location_sql = &sql_para.location_sql;

    //构造函数中计时器不启动
    if(index == menu_index){
        timer->setInterval(sql_para.location_sql.time * 1000);
        fresh_setting();
        timer2->start();
    }
}

void FaultLocation::trans_key(quint8 key_code)
{
    if (key_val == NULL || key_val->grade.val0 != menu_index) {
        return;
    }

    switch (key_code) {
    case KEY_OK:
        if(key_val->grade.val2 == 5 ){
            saveCurrentData();
        }
        else{
            timer->start();
            startANewScan();
            key_val->grade.val1 = 2;
        }
        sqlcfg->sql_save(&sql_para);     //保存设置

        key_val->grade.val2 = 0;
        break;
    case KEY_CANCEL:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        sql_para = *sqlcfg->get_para();
        timer->stop();
        processingAScan();
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
    fresh();
}

void FaultLocation::do_key_up_down(int d)
{
    if(key_val->grade.val1 == 2){        //控制图形
        if(d>0){
            scale = scale / 1.2;
        }
        else{
            scale = scale * 1.2;
        }
    }
    else{
        key_val->grade.val1 = 1;
        Common::change_index(key_val->grade.val2, d, SETTING_NUM, 1);
    }
}

void FaultLocation::do_key_left_right(int d)
{
    if(key_val->grade.val1 == 2){        //控制图形
        Common::change_index(x, d*100, wave1.length() - 300, 0);
    }
    else{
        switch (key_val->grade.val2) {
        case 1:
            sql_para.location_sql.mode = !sql_para.location_sql.mode;
            break;
        case 2:
            Common::change_value(sql_para.location_sql.chart_mode, CURVE, CURVE);
            break;
        case 3:
            Common::change_index(sql_para.location_sql.channel, d, Double, Left);
            break;
        case 4:
            Common::change_index(sql_para.location_sql.time, d, 10, 1);
            timer->setInterval(sql_para.location_sql.time * 1000);
            break;
        default:
            break;
        }
    }
}

//收到双通道录波数据
void FaultLocation::showWaveData(VectorList buf, MODE)
{
    if(key_val->grade.val0 == menu_index){
        qDebug()<<"double channel data received!    " << buf.length();
        data->set_send_para (sp_rec_on, 0);
        data->set_send_para (sp_auto_rec, 0);

        setData(buf);
    }
}

void FaultLocation::setData(VectorList buf)
{
    currentData = buf;

    QPointF p;
    wave1.clear();
    wave2.clear();
    max=0;
    min=0;
    double v_real = 0;
    for (int i = 0; i < buf.length(); ++i) {
        if(i < buf.length()/2){     //TEV1
            v_real = sqlcfg->get_para()->tev1_sql.gain * TEV_FACTOR * buf.at(i);
            p = QPointF(i*0.01, v_real);
            wave1.append(p);
        }
        else{                       //TEV2
            v_real = sqlcfg->get_para()->tev2_sql.gain * TEV_FACTOR * buf.at(i);
            p = QPointF((i-buf.length()/2)*0.01, v_real);
            wave2.append(p);
        }

        if(v_real>max){
            max = v_real;
        }
        else if(v_real<min){
            min = v_real;
        }
    }

    if(min > -SHOW_FACTOR){
        min = -SHOW_FACTOR;
    }
    if(max < SHOW_FACTOR ){
        max = SHOW_FACTOR;
    }

    plot->setAxisScale(QwtPlot::yLeft, min, max);



    x = 200;   //开始显示最左边数据
    scale = 1.0;    //纵坐标拉伸因子为1

    fresh();
}

void FaultLocation::get_origin_points(QVector<QPoint> p, int group)
{

    if(group <= 3 && group >=0){
        groupNum_left = group;
        points_left = p;
//        qDebug()<<"FaultLocation get data! Left  " << groupNum_left <<"\t"<<  points_left.length();
    }
    else if(group >=4 && group <=7){
        groupNum_right = group;
        points_right = p;
//        qDebug()<<"FaultLocation get data! Right  "<< groupNum_right <<"\t"<<  points_right.length();;
    }
//    compare();

}

void FaultLocation::saveCurrentData()
{
    qDebug()<<"save current graph!";
    filetools = new FileTools(currentData,Double_Channel,FileTools::Write);      //开一个线程，为了不影响数据接口性能
    QThreadPool::globalInstance()->start(filetools);
}

void FaultLocation::setScroll(int value)
{
    if(wave1.length()-value > 300 / X_SCALE){
        QVector<QPointF> w1,w2;
        w1 = wave1.mid(value,300 / X_SCALE);
        w2 = wave2.mid(value,300 / X_SCALE);
        curve1->setSamples(w1);
        curve2->setSamples(w2);
        plot->setAxisScale(QwtPlot::xBottom, w1.first().x(), w1.last().x());
        plot->replot();
    }
}

void FaultLocation::fresh()
{
    this->setScroll(x);
    plot->setAxisScale(QwtPlot::yLeft, min * scale , max * scale);

    plot->replot();
}

void FaultLocation::setCompassValue()
{

    int a = qrand()%180 - 90; //范围是-90~90
//    qDebug()<<"a="<<a;

    to = 360+a;              //范围是270~450

    process = 1;
}

void FaultLocation::setCompassValue(int c)
{
    int a = c; //范围是-90~90

    to = 360+a;              //范围是270~450

    process = 1;
}

void FaultLocation::setMiniCompassValue()
{
    if(process){
        ui->Compass->setValue( ((int)(from + process * (to - from) * 1.0 / speed )) % 360 );
        process ++ ;        //process范围是1-11
    }

    if(process == speed +1 ){
        from = to;
        process = 0;
    }
}

//功能
//1.如果处于连续扫描模式，则开启下一次扫描
//2.如果未得到触发信号，则开启一次手动录波
void FaultLocation::processingAScan()
{    
    if(timer->isActive()){
        startANewScan();
    }
    else{
//        data->set_send_para (sp_auto_rec, sql_para.tev1_sql.auto_rec + 2 * sql_para.tev2_sql.auto_rec);   //恢复自动录波
        data->set_send_para (sp_rec_on, 0);
        data->set_send_para (sp_auto_rec, 0);           //关闭自动录波
    }
}

void FaultLocation::startANewScan()
{
    qDebug()<<"start a new scan";
//    timer2->start();
    if(sql_para.location_sql.channel == Left){
        data->set_send_para (sp_rec_on, 1);
        data->set_send_para (sp_auto_rec, 12);
    }
    else if(sql_para.location_sql.channel == Right){
        data->set_send_para (sp_rec_on, 1);
        data->set_send_para (sp_auto_rec, 20);
    }
    else if(sql_para.location_sql.channel == Double){
        data->set_send_para (sp_rec_on, 1);
        data->set_send_para (sp_auto_rec, 28);
    }

}

void FaultLocation::showProgress()
{
    //刷新前两通道的读数(放在这里是暂时借用,以后视情况开新计时器)
    yc_data_type tev1_a, tev2_a, tev1_p, tev2_p, hf1_a, hf2_a, hf1_p, hf2_p;
    unsigned char a[1],b[1];

    yc_get_value(0,TEV1_amplitude,1, &tev1_a, b, a);
    yc_get_value(0,TEV1_num,1, &tev1_p, b, a);
    yc_get_value(0,TEV2_amplitude,1, &tev2_a, b, a);
    yc_get_value(0,TEV2_num,1, &tev2_p, b, a);
    yc_get_value(0,HFCT1_amplitude,1, &hf1_a, b, a);
    yc_get_value(0,HFCT1_num,1, &hf1_p, b, a);
    yc_get_value(0,HFCT2_amplitude,1, &hf2_a, b, a);
    yc_get_value(0,HFCT2_num,1, &hf2_p, b, a);

    switch (mode_list.at(0)) {
    case TEV1:
        ui->label_DB_1->setText(QString("%1").arg(tev1_a.f_val));
        ui->label_Pulse_1->setText(QString("%1").arg(tev1_p.f_val));
        break;
    case TEV2:
        ui->label_DB_1->setText(QString("%1").arg(tev2_a.f_val));
        ui->label_Pulse_1->setText(QString("%1").arg(tev2_p.f_val));
        break;
    case HFCT1:
        ui->label_DB_1->setText(QString("%1").arg(hf1_a.f_val));
        ui->label_Pulse_1->setText(QString("%1").arg(hf1_p.f_val));
        break;
    case HFCT2:
        ui->label_DB_1->setText(QString("%1").arg(hf2_a.f_val));
        ui->label_Pulse_1->setText(QString("%1").arg(hf2_p.f_val));
        break;
    default:
        break;
    }

    switch (mode_list.at(1)) {
    case TEV1:
        ui->label_DB_2->setText(QString("%1").arg(tev1_a.f_val));
        ui->label_Pulse_2->setText(QString("%1").arg(tev1_p.f_val));
        break;
    case TEV2:
        ui->label_DB_2->setText(QString("%1").arg(tev2_a.f_val));
        ui->label_Pulse_2->setText(QString("%1").arg(tev2_p.f_val));
        break;
    case HFCT1:
        ui->label_DB_2->setText(QString("%1").arg(hf1_a.f_val));
        ui->label_Pulse_2->setText(QString("%1").arg(hf1_p.f_val));
        break;
    case HFCT2:
        ui->label_DB_2->setText(QString("%1").arg(hf2_a.f_val));
        ui->label_Pulse_2->setText(QString("%1").arg(hf2_p.f_val));
        break;
    default:
        break;
    }

    if(timer->isActive()){
        int n = 100 - timer->remainingTime() / sql_para.location_sql.time / 10.0;
        ui->progressBar1->setValue( n );
        ui->progressBar2->setValue( n );
        ui->progressBar1->repaint();
        ui->progressBar2->repaint();
    }
}

void FaultLocation::compare()
{
    if(groupNum_left + 4 == groupNum_right && points_left.length()>0 && points_right.length()>0){
        qDebug()<<"get valid data!\t"<<points_left.length()<<"\t"<<points_right.length();
        int c1 = 0, c2 =0, n = 0;
        foreach (QPoint p1, points_left) {
            foreach (QPoint p2, points_right) {
                if(qAbs(p1.x()-p2.x())<3 && p1.y()*p2.y()>0){
                    qDebug()<<"found close point pair!   "<<p1<<p2 << "\t"<<p1.x()-p2.x();
                    c1 += qAbs(p1.y() );
                    c2 += qAbs(p2.y() );
                    n++;        //计数器
                }
            }
        }

        if(n>0){
            if( (points_left.length()-points_right.length() )* (c1 - c2) > 0 ){
                qDebug()<< "c1=" << c1 * 1.0 / n <<"\tc2="<< c2 * 1.0 / n << "\tc1-c2="<< (c1 - c2)*1.0 / n;
                setCompassValue(-10*(c1 - c2)/ n);
            }
        }
    }
}

void FaultLocation::fresh_setting()
{
//    qDebug()<<"key_val->grade.val0 is :"<<key_val->grade.val0;
//    qDebug()<<"key_val->grade.val1 is :"<<key_val->grade.val1;
//    qDebug()<<"key_val->grade.val2 is :"<<key_val->grade.val2;

    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == menu_index) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    if(sql_para.location_sql.mode == single){
        ui->comboBox->setItemText(0,tr("触发模式\t[单次]"));
    }
    else{
        ui->comboBox->setItemText(0,tr("触发模式\t[连续]"));
    }
    if(sql_para.location_sql.chart_mode == COMPASS){
        ui->comboBox->setItemText(1,tr("显示模式\t[罗盘]"));
        ui->Compass->show();
        plot->hide();
    }
    else{
        ui->comboBox->setItemText(1,tr("显示模式\t[波形]"));
        plot->show();
        ui->Compass->hide();
    }

    if(sql_para.location_sql.channel == Left){
        ui->comboBox->setItemText(2,tr("触发通道\t[%1]").arg(Common::MODE_toString(mode_list.at(0)) ));
    }
    else if(sql_para.location_sql.channel == Right){
        ui->comboBox->setItemText(2,tr("触发通道\t[%1]").arg(Common::MODE_toString(mode_list.at(1)) ));
    }
    else {
        ui->comboBox->setItemText(2,tr("触发通道\t[双]"));
    }
    ui->comboBox->setItemText(3,tr("触发时长\t[%1]s").arg(sql_para.location_sql.time));

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));


    timer->setSingleShot(!sql_para.location_sql.mode);
    emit fresh_parent();

}






