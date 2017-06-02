#include "tevwidget.h"
#include "ui_amplitude1.h"
#include <QTimer>
#include <QLineEdit>
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


#define VALUE_MAX  60           //RPPD最大值
#define SETTING_NUM 8           //设置菜单条目数



TEVWidget::TEVWidget(G_PARA *data, Channel channel, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::Amplitude1)
{
    ui->setupUi(this);
    this->resize(455, 185);
    this->move(2, 31);
    this->setStyleSheet("TEVWidget {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");

    key_val = NULL;
    /* get sql para */
    sql_para = sqlcfg->get_para();

    this->channel = channel;
    if(channel == Left){
        tev_sql = &sql_para->tev1_sql;
    }
    else{
        tev_sql = &sql_para->tev2_sql;
    }


    this->data = data;
    db = 0;
    max_db = 0;

    pulse_cnt_last = 0;


    QLineEdit *lineEdit = new QLineEdit;
    ui->comboBox->setStyleSheet("QComboBox {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray; }"
//                                "QComboBox::down-arrow {image: url(:/widgetphoto/mainmenu/down.png}"
//                                "QComboBox::drop-down {width: 30px;}"
                                );
    ui->comboBox->setLineEdit(lineEdit);
    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
    ui->comboBox->lineEdit()->setReadOnly(true);
    ui->comboBox->lineEdit()->setStyleSheet("QLineEdit {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray}");
    ui->comboBox->view()->setStyleSheet("QListView {border-image: url(:/widgetphoto/mainmenu/para_child.png);color:gray;outline: none;}");



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

//    if(channel == Left){
//        timer3 = new QTimer;
//        timer3->setInterval(45);
//        connect(timer3,SIGNAL(timeout()),this,SLOT(change_channel()));

//        timer3->start();
//        channel_flag = true;
//    }

    recWaveForm = new RecWaveForm(this);
    recWaveForm->hide();
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));
}

TEVWidget::~TEVWidget()
{
    delete ui;
}

void TEVWidget::showWaveData(VectorList buf, MODE mod)
{
    key_val->grade.val1 = 1;        //为了锁住主界面，防止左右键切换通道
    key_val->grade.val5 = 1;
    recWaveForm->working(key_val,buf,mod);
}

void TEVWidget::PRPS_inti()
{
    plot_PRPS = new QwtPlot(ui->widget);
    plot_PRPS->resize(200, 140);
    plot_PRPS->setStyleSheet("background:transparent;color:gray;");

    plot_PRPS->setAxisScale(QwtPlot::xBottom, 0, 13);
    plot_PRPS->setAxisScale(QwtPlot::yLeft, 0, 60, 20);

    plot_PRPS->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot_PRPS->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    //plot->setAxisMaxMajor(QwtPlot::yLeft, 2);

    plot_PRPS->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot_PRPS->axisWidget(QwtPlot::yLeft)->setMargin(0);
    plot_PRPS->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot_PRPS->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot_PRPS->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    plot_PRPS->plotLayout()->setAlignCanvasToScales(true);

    d_PRPS = new BarChart(plot_PRPS, &db, &tev_sql->high, &tev_sql->low);
    connect(timer1, &QTimer::timeout, d_PRPS, &BarChart::fresh);
}

void TEVWidget::PRPD_inti()
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

void TEVWidget::histogram_init()
{
    plot_Histogram = new QwtPlot(ui->widget);
    plot_Histogram->resize(200, 150);
    plot_Histogram->setStyleSheet("background:transparent;color:gray;");

    plot_Histogram->setAxisScale(QwtPlot::xBottom, -60, 60);
    plot_Histogram->setAxisScale(QwtPlot::yLeft, 0, 50);
//    plot_Histogram->setAxisScale(QwtPlot::xBottom, 0, 6);
//    plot_Histogram->setAxisScale(QwtPlot::yLeft, 0.01, 10);

    plot_Histogram->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot_Histogram->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    /* remove gap */
    plot_Histogram->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot_Histogram->axisWidget(QwtPlot::xBottom)->setTitle("dB");
    plot_Histogram->axisWidget(QwtPlot::yLeft)->setMargin(0);

//    plot_Histogram->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );  //对数坐标
    plot_Histogram->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot_Histogram->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot_Histogram->plotLayout()->setAlignCanvasToScales(true);

    d_histogram = new QwtPlotHistogram;
    d_histogram->setStyle( QwtPlotHistogram::Columns );
    d_histogram->setBrush(Qt::green);
    d_histogram->setPen(Qt::green);
    d_histogram->attach(plot_Histogram);

}

void TEVWidget::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    this->show();
}

void TEVWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if(channel == Left && key_val->grade.val0 != 0){
        return;
    }

    if(channel == Right && key_val->grade.val0 != 1){
        return;
    }

    if(key_val->grade.val5 != 0){
        emit send_key(key_code);
        return;
    }


    switch (key_code) {
    case KEY_OK:
        sqlcfg->sql_save(sql_para);
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
            if (tev_sql->mode_chart == PRPS) {
                tev_sql->mode_chart = Histogram;
                break;
            } else if(tev_sql->mode_chart == Histogram){
                tev_sql->mode_chart = PRPD;
                break;
            } else if(tev_sql->mode_chart == PRPD){
                tev_sql->mode_chart = PRPS;
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
                tev_sql->mode_chart = PRPS;
                break;
            } else if (tev_sql->mode_chart == PRPS) {
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
	
	if (channel == Left) {
		d_max = data->recv_para.hdata0.ad.ad_max;
		d_min = data->recv_para.hdata0.ad.ad_min;
    }
	else{
		d_max = data->recv_para.hdata1.ad.ad_max;
		d_min = data->recv_para.hdata1.ad.ad_min;
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

void TEVWidget::fresh_plot()
{
    double t, s, degree;
    quint32 pulse_cnt,pulse_cnt_show;      //脉冲计数
    int sug_central_offset, sug_offset;
//    quint32 signal_pulse_cnt;

    if (tev_sql->mode == single) {
        timer1->stop();      //如果单次模式，停止计时器
    }

#if 0
    double a,b;
    a = d_max - 0x8000;
    b = d_min - 0x8000;
    emit offset_suggest((int)(a/10),(int)(b/10));
        qDebug()<<"[1]a = "<<a <<"\tb = "<<b << "a-b="<<d_max-d_min;

    a = AD_VAL(d_max, (0x8000+tev_sql->tev_offset1*10) );
    b = AD_VAL(d_min, (0x8000+tev_sql->tev_offset2*10) );

    t = tev_sql->tev_gain*((double)MAX(a, b) * 1000) / 32768;
    s = ((double)20) * log10(t);      //对数运算，来自工具链的函数
#else
	calc_tev_value (&t, &s, &sug_central_offset, &sug_offset);
    emit offset_suggest (sug_central_offset, sug_offset);
#endif

    //记录并显示最大值
    if (max_db < s) {
        max_db = s;
        ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "dB");
    }

    //脉冲计数和严重度
//    signal_pulse_cnt = data->recv_para.pulse1.edge.neg + data->recv_para.pulse1.edge.pos;
    if(channel == Left){
        pulse_cnt = data->recv_para.hpulse0_totol;
    }
    else{
        pulse_cnt = data->recv_para.hpulse1_totol;
    }

    pulse_cnt_show = pulse_cnt_last + pulse_cnt;    //显示2秒的脉冲计数
    pulse_cnt_last = pulse_cnt;

    degree = pow(t,tev_sql->gain) * (double)pulse_cnt / sql_para->freq_val;     //严重度算法更改

    ui->label_pluse->setText(tr("脉冲数: ") + QString::number(pulse_cnt_show));
    ui->label_degree->setText(tr("严重度: ") + QString::number(degree, 'f', 2));

    ui->label_val->setText(QString::number((qint16)s));

    db = (int)s;

    emit tev_modbus_data(db,pulse_cnt_show);

    if ( db > tev_sql->high) {
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
    } else if(tev_sql->mode_chart == PRPS){
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
    int x,y,len;
//    qDebug()<<"data->recv_para.recData[0] = "<<data->recv_para.recData[0];
    if(groupNum != data->recv_para.recData[0] && data->recv_para.recComplete == 0){     //有效数据
        if( (channel == Left && data->recv_para.recData[0] < 4) || (channel == Right && data->recv_para.recData[0] >= 4) ){
            groupNum = data->recv_para.recData[0];
            //处理数据

            points_origin.clear();

            len = data->recv_para.recData[1];
            if(len == 0){       //无脉冲时，只读2数据

            }
            else{               //有脉冲时，不读底噪
                for(int i=0;i<len;i++){
                    x = (int)data->recv_para.recData[2*i+4];
                    y = (int)data->recv_para.recData[2*i+5];

                    transData(x,y);
                }
            }
            d_PRPD->setSamples(points);

            plot_PRPD->replot();

            emit origin_pluse_points(points_origin, groupNum);
        }
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

//void TEVWidget::change_channel()
//{
//    //每隔一段时间，改变组号，为了改变FPGA发送的脉冲数据通道
//    if(data->recv_para.recComplete){
//        if(channel_flag){
//            data->send_para.groupNum.rval = 0x0100;
//        }
//        else{
//            data->send_para.groupNum.rval = 0x0000;
//        }


//        channel_flag = !channel_flag;
//        data->send_para.groupNum.flag = true;
//    }

//}

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
    if(x>360){
        qDebug()<<"x="<<x;
    }


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
    points.clear();
    fresh_PRPD();
}

void TEVWidget::rec_wave()
{
    if(channel == Left){
        emit startRecWave(0,0);
    }
    else{
        emit startRecWave(1,0);
    }
}

void TEVWidget::maxReset()
{
    max_db = 0;
    ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "dB");
}

void TEVWidget::fresh_setting()
{
    if (tev_sql->mode == single) {
        ui->comboBox->setItemText(0,tr("检测模式    [单次]"));
    } else {
        ui->comboBox->setItemText(0,tr("检测模式    [连续]"));
    }
    if (tev_sql->mode_chart == PRPD) {
        ui->comboBox->setItemText(1,tr("图形显示    [PRPD]"));
        plot_PRPD->show();
        plot_PRPS->hide();
        plot_Histogram->hide();
    } else if(tev_sql->mode_chart == PRPS){
        ui->comboBox->setItemText(1,tr("图形显示    [PRPS]"));
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

    if(channel == Left){
        if (key_val->grade.val2 && key_val->grade.val0 ==0) {
            ui->comboBox->showPopup();
        }
        else{
            ui->comboBox->hidePopup();
        }
    }
    else if(channel == Right){
        if (key_val->grade.val2 && key_val->grade.val0 ==1) {
            ui->comboBox->showPopup();
        }
        else{
            ui->comboBox->hidePopup();
        }
    }
    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
}

