#include "faultlocation.h"
#include "ui_faultlocation.h"
#include <QLineEdit>
#include <qwt_dial_needle.h>
#include <QTimer>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include "IO/Data/filetools.h"
#include <QThreadPool>

#define SHOW_FACTOR 4.0     //保存默认的也是最小的纵轴间距

#define X_SCALE 2.0     //X拉伸因子

FaultLocation::FaultLocation(G_PARA *data, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FaultLocation)
{
    ui->setupUi(this);

    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("FaultLocation {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");

    key_val = NULL;

    sql_para = *sqlcfg->get_para();

    this->data = data;

    QLineEdit *lineEdit = new QLineEdit;
    ui->comboBox->setStyleSheet("QComboBox {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray; }");
    ui->comboBox->setLineEdit(lineEdit);
    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    ui->comboBox->lineEdit()->setReadOnly(true);
    ui->comboBox->lineEdit()->setStyleSheet("QLineEdit {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    ui->comboBox->view()->setStyleSheet("QListView {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray;outline: none;}");


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

    //脉冲触发计时器
    timer = new QTimer;
    connect(timer,SIGNAL(timeout()),this,SLOT(processingAScan()) );

    //进度条
    timer2 = new QTimer;
    timer2->setInterval(10);
//    timer2->setSingleShot(true);
    connect(timer2,SIGNAL(timeout()),this,SLOT(showProgress()));

    //绘图
    plot = new QwtPlot(this);
    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setSpacing(10);
    plot->axisWidget(QwtPlot::xBottom)->setTitle("us");
    plot->axisWidget(QwtPlot::yLeft)->setTitle("V m");

    plot->resize(300,150);
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
}

FaultLocation::~FaultLocation()
{
    delete ui;
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

void FaultLocation::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    this->show();

    //根据sql信息初始化设置菜单
//    reloadSql();        //重置默认数据
    timer->setInterval(sql_para.location_sql.time * 1000);
    fresh_setting();
}

void FaultLocation::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if(key_val->grade.val0 != 2){
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
        if(key_val->grade.val1 == 2){        //控制图形
            scale = scale * 1.2;
        }
        else{
            if (key_val->grade.val2 < 2) {
                key_val->grade.val2 = 5;
            } else {
                key_val->grade.val2--;
            }
        }
        break;
    case KEY_DOWN:
        if(key_val->grade.val1 == 2){        //控制图形
            scale = scale / 1.2;
        }
        else{
            if (key_val->grade.val2 > 4) {
                key_val->grade.val2 = 1;
            } else {
                key_val->grade.val2++;
            }
        }
        break;
    case KEY_LEFT:
        if(key_val->grade.val1 == 2){        //控制图形
            if(x<100){
                x = 0;
            }
            else{
                x = x -100;
            }
        }
        else{
            switch (key_val->grade.val2) {
            case 1:
                sql_para.location_sql.mode = !sql_para.location_sql.mode;
                break;
            case 2:
                if( sql_para.location_sql.chart_mode == COMPASS ){
                    sql_para.location_sql.chart_mode = CURVE;
                }
                else{
                    sql_para.location_sql.chart_mode = COMPASS;
                }
                break;
            case 3:
                if( sql_para.location_sql.channel == Left ){
                    sql_para.location_sql.channel = Double;
                }
                else if(sql_para.location_sql.channel == Right){
                    sql_para.location_sql.channel = Left;
                }
                else if(sql_para.location_sql.channel == Double){
                    sql_para.location_sql.channel = Right;
                }
                break;
            case 4:
                if(sql_para.location_sql.time > 2){
                    sql_para.location_sql.time -= 2;
                    timer->setInterval(sql_para.location_sql.time * 1000);
                }
                break;
            default:
                break;
            }
        }

        break;
    case KEY_RIGHT:
        if(key_val->grade.val1 == 2){        //控制图形
            if(x<wave1.length()-400){
                x = x +100;
            }
            else{
                x = wave1.length() - 300;
            }
        }
        else{
            switch (key_val->grade.val2) {
            case 1:
                sql_para.location_sql.mode = !sql_para.location_sql.mode;
                break;
            case 2:
                if( sql_para.location_sql.chart_mode == COMPASS ){
                    sql_para.location_sql.chart_mode = CURVE;
                }
                else{
                    sql_para.location_sql.chart_mode = COMPASS;
                }
                break;
            case 3:
                if( sql_para.location_sql.channel == Left ){
                    sql_para.location_sql.channel = Right;
                }
                else if(sql_para.location_sql.channel == Right){
                    sql_para.location_sql.channel = Double;
                }
                else if(sql_para.location_sql.channel == Double){
                    sql_para.location_sql.channel = Left;
                }
                break;
            case 4:
                if(sql_para.location_sql.time < 9){
                    sql_para.location_sql.time += 2;
                    timer->setInterval(sql_para.location_sql.time * 1000);
                }
                break;
            default:
                break;
            }
        }

        break;
    default:
        break;
    }
    fresh_setting();
    fresh();
}

void FaultLocation::showLeftData(int db, int p)
{
    ui->label_DB_1->setText(QString("%1").arg(db));
    ui->label_Pulse_1->setText(QString("%1").arg(p));
}

void FaultLocation::showRightData(int db, int p)
{
    ui->label_DB_2->setText(QString("%1").arg(db));
    ui->label_Pulse_2->setText(QString("%1").arg(p));
}

//收到双通道录波数据
void FaultLocation::showWaveData(VectorList buf, MODE)
{
    qDebug()<<"double channel data received!    " << buf.length();
    data->set_send_para (sp_tev_auto_rec, 0);

    setData(buf);
//    data->set_send_para (sp_recstart_ad1, 0);
//    data->set_send_para (sp_recstart_ad2, 0);
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


void FaultLocation::saveCurrentData()
{
    qDebug()<<"save current graph!";
    filetools = new FileTools(currentData,TEV_Double);      //开一个线程，为了不影响数据接口性能
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
////2.如果未得到触发信号，则开启一次手动录波
void FaultLocation::processingAScan()
{    
//    if(sql_para.location_sql.mode){    //连续模式
    if(timer->isActive()){
        startANewScan();
    }
    else{
        data->set_send_para (sp_tev_auto_rec, sql_para.tev1_sql.auto_rec + 2 * sql_para.tev2_sql.auto_rec);   //恢复自动录波
    }
}

void FaultLocation::startANewScan()
{
    qDebug()<<"start a new scan";
    timer2->start();
    if(sql_para.location_sql.channel == Left){
        data->set_send_para (sp_tev_auto_rec, 12);
    }
    else if(sql_para.location_sql.channel == Right){
        data->set_send_para (sp_tev_auto_rec, 20);
    }
    else if(sql_para.location_sql.channel == Double){
        data->set_send_para (sp_tev_auto_rec, 28);
    }

}

void FaultLocation::showProgress()
{
    if(timer->isActive()){
        int n = 100 - timer->remainingTime() / sql_para.location_sql.time / 10;
        ui->progressBar1->setValue( n );
        ui->progressBar2->setValue( n );
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

    if (key_val->grade.val2 && key_val->grade.val0 == 2) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    if(sql_para.location_sql.mode == single){
        ui->comboBox->setItemText(0,tr("触发模式    [单次]"));
    }
    else{
        ui->comboBox->setItemText(0,tr("触发模式    [连续]"));
    }
    if(sql_para.location_sql.chart_mode == COMPASS){
        ui->comboBox->setItemText(1,tr("显示模式    [罗盘]"));
        ui->Compass->show();
        plot->hide();
    }
    else{
        ui->comboBox->setItemText(1,tr("显示模式    [波形]"));
        plot->show();
        ui->Compass->hide();
    }

    if(sql_para.location_sql.channel == Left){
        ui->comboBox->setItemText(2,tr("触发通道    [1]"));
    }
    else if(sql_para.location_sql.channel == Right){
        ui->comboBox->setItemText(2,tr("触发通道    [2]"));
    }
    else {
        ui->comboBox->setItemText(2,tr("触发通道    [双]"));
    }
    ui->comboBox->setItemText(3,tr("触发时长    [%1]s").arg(sql_para.location_sql.time));

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));


    timer->setSingleShot(!sql_para.location_sql.mode);
    emit fresh_parent();

}






