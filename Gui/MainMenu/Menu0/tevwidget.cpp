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
        amp_sql = &sql_para->amp_sql1;
    }
    else{
        amp_sql = &sql_para->amp_sql2;
    }


    this->data = data;
    db = 0;
    max_db = 0;




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
    if (amp_sql->mode == series) {
        timer1->start();
    }
    connect(timer1, SIGNAL(timeout()), this, SLOT(fresh_plot()));
    connect(timer1, SIGNAL(timeout()), this, SLOT(fresh_Histogram()));

    timer2 = new QTimer(this);
    timer2->setInterval(100);
    connect(timer2, SIGNAL(timeout()), this, SLOT(fresh_PRPD()));
    timer2->start();

    PRPS_inti();
    PRPD_inti();
    histogram_init();
}

TEVWidget::~TEVWidget()
{
    delete ui;
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

    d_PRPS = new BarChart(plot_PRPS, &db, amp_sql->high, amp_sql->low);
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


void TEVWidget::sysReset()
{
    fresh_setting();
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


    switch (key_code) {
    case KEY_OK:
//        memcpy(&sql_para->amp_sql, amp_sql, sizeof(AMP_SQL));
        sqlcfg->sql_save(sql_para);
        timer1->start();                                                         //and timer no stop
        if(key_val->grade.val2 == 5){
            this->maxReset();
        }
        if(key_val->grade.val2 == 6){
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
            key_val->grade.val2 = 6;
        } else {
            key_val->grade.val2--;
        }
        break;
    case KEY_DOWN:
        if (key_val->grade.val2 > 5) {
            key_val->grade.val2 = 1;
        } else {
            key_val->grade.val2++;
        }
        break;
    case KEY_LEFT:
        switch (key_val->grade.val2) {
        case 1:
            if (amp_sql->mode == signal) {
                amp_sql->mode = series;
            } else {
                amp_sql->mode = signal;
            }
            break;
        case 2:
            if (amp_sql->mode_chart == PRPS) {
                amp_sql->mode_chart = Histogram;
                break;
            } else if(amp_sql->mode_chart == Histogram){
                amp_sql->mode_chart = PRPD;
                break;
            } else if(amp_sql->mode_chart == PRPD){
                amp_sql->mode_chart = PRPS;
                break;
            }

        case 3:
            if (amp_sql->high > amp_sql->low) {
                amp_sql->high--;
            }
            break;
        case 4:
            if (amp_sql->low > 0) {
                amp_sql->low--;
            }
            break;
        default:
            break;
        }
        break;

    case KEY_RIGHT:
        switch (key_val->grade.val2) {
        case 1:
            if (amp_sql->mode == signal) {
                amp_sql->mode = series;
            } else {
                amp_sql->mode = signal;
            }
            break;
        case 2:
            if (amp_sql->mode_chart == Histogram) {
                amp_sql->mode_chart = PRPS;
                break;
            } else if (amp_sql->mode_chart == PRPS) {
                amp_sql->mode_chart = PRPD;
                break;
            } else if(amp_sql->mode_chart == PRPD){
                amp_sql->mode_chart = Histogram;
                break;
            }
        case 3:
            if (amp_sql->high < 60) {
                amp_sql->high++;
            }
            break;
        case 4:
            if (amp_sql->low < amp_sql->high) {
                amp_sql->low++;
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

void TEVWidget::fresh_plot()
{
    double t, s, degree;
    quint32 pulse_cnt;      //脉冲计数
//    quint32 signal_pulse_cnt;

    if (amp_sql->mode == signal) {
        timer1->stop();      //如果单次模式，停止计时器
    }

    double a,b;

    double d_max,d_min;
    if(channel == Left){
        d_max = data->recv_para.hdata0.ad.ad_max;
        d_min = data->recv_para.hdata0.ad.ad_min;
    }
    else{
        d_max = data->recv_para.hdata1.ad.ad_max;
        d_min = data->recv_para.hdata1.ad.ad_min;
    }

    a = d_max - 0x8000;
    b = d_min - 0x8000;
    emit offset_suggest((int)(a/10),(int)(b/10));
//        qDebug()<<"[1]a = "<<a <<"\tb = "<<b;

    a = AD_VAL(d_max, (0x8000+amp_sql->tev_offset1*10) );
    b = AD_VAL(d_min, (0x8000+amp_sql->tev_offset2*10) );

    t = ((double)MAX(a, b) * 1000) / 32768;
    s = amp_sql->tev_gain*((double)20) * log10(t);      //对数运算，来自工具链的函数

    //记录并显示最大值
    if (max_db < s) {
        max_db = s;
        ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "dB");
    }

    //脉冲计数和严重度
//    signal_pulse_cnt = data->recv_para.pulse1.edge.neg + data->recv_para.pulse1.edge.pos;
    pulse_cnt = data->recv_para.hpulse0_totol;
    degree = s * (double)pulse_cnt / 1000;

    ui->label_pluse->setText(tr("脉冲数: ") + QString::number(pulse_cnt));
    ui->label_degree->setText(tr("严重度: ") + QString::number(degree, 'f', 2));

    ui->label_val->setText(QString::number((qint16)s));

    db = (int)s;

    if ( db > amp_sql->high) {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:red}");
    } else if (db >= amp_sql->low) {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:yellow}");
    } else {
        ui->label_val->setStyleSheet("QLabel {font-family:WenQuanYi Micro Hei;font-size:60px;color:green}");
    }

    if (amp_sql->mode_chart == PRPD) {
        plot_PRPD->show();
        plot_PRPS->hide();
        plot_Histogram->hide();
        plot_PRPD->replot();
    } else if(amp_sql->mode_chart == PRPS){
        plot_PRPD->hide();
        plot_PRPS->show();
        plot_Histogram->hide();
        plot_PRPS->replot();
    } else if(amp_sql->mode_chart == Histogram){
        plot_PRPD->hide();
        plot_PRPS->hide();
        plot_Histogram->show();
        plot_Histogram->replot();
    }
}

void TEVWidget::fresh_PRPD()
{
    int x,y,len;
    if(groupNum != data->recv_para.recData[0] && data->recv_para.recComplete == 0){     //有效数据
        groupNum = data->recv_para.recData[0];
        //处理数据
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

    //    if(temp != points.length()){
    //        temp = points.length();
    //        qDebug()<<"points num : "<<points.length();
    //    }

        plot_PRPD->replot();

//        if(len != 0)
//            qDebug()<<"read " << len <<  " PRPD data !";
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
//    qDebug()<<"[1]x="<<x<<"\ty="<<y;

    y = y - 0x8000;
    if(y>0){
        y = y - amp_sql->tev_offset1;
    }
    else{
        y = y - amp_sql->tev_offset2;
    }

    y = (int)(((double)y * 1000) / 32768);
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

void TEVWidget::maxReset()
{
    max_db = 0;
    ui->label_max->setText(tr("最大值: ") + QString::number(max_db) + "dB");
}

void TEVWidget::fresh_setting()
{
    if (amp_sql->mode == signal) {
        ui->comboBox->setItemText(0,tr("检测模式    [单次]"));
    } else {
        ui->comboBox->setItemText(0,tr("检测模式    [连续]"));
    }
    if (amp_sql->mode_chart == PRPD) {
        ui->comboBox->setItemText(1,tr("图形显示    [PRPD]"));
        plot_PRPD->show();
        plot_PRPS->hide();
        plot_Histogram->hide();
    } else if(amp_sql->mode_chart == PRPS){
        ui->comboBox->setItemText(1,tr("图形显示    [PRPS]"));
        plot_PRPD->hide();
        plot_PRPS->show();
        plot_Histogram->hide();
    } else if(amp_sql->mode_chart == Histogram){
        ui->comboBox->setItemText(1,tr("图形显示  [柱状图]"));
        plot_PRPD->hide();
        plot_PRPS->hide();
        plot_Histogram->show();
    }
    ui->comboBox->setItemText(2,tr("黄色报警阈值[%1]dB").arg(QString::number(amp_sql->high)));
    ui->comboBox->setItemText(3,tr("红色报警阈值[%1]dB").arg(QString::number(amp_sql->low)));


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

