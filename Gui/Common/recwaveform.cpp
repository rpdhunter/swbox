#include "recwaveform.h"
#include <QtDebug>
#include <QFile>
#include <QScrollBar>
#include "IO/Other/filetools.h"
#include <QThreadPool>

#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_plot_marker.h>

#include "IO/SqlCfg/sqlcfg.h"
#include "common.h"

#define SHOW_FACTOR 4.0     //保存默认的也是最小的纵轴间距
#define PEAK_JUDGE  4.0     //脉冲阈值阈值


RecWaveForm::RecWaveForm(int menu_index, QWidget *parent) :
    QWidget(parent)
{
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->setStyleSheet("background:white;");

    this->menu_index = menu_index;
    plot = new QwtPlot(this);
    plot->resize(CHANNEL_X,CHANNEL_Y);

    curve1 = new QwtPlotCurve();
    curve1->setPen(QPen(Qt::darkBlue, 0, Qt::SolidLine, Qt::RoundCap));
    curve1->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve1->setTitle("Channel 1");
    curve1->attach(plot);

    curve2 = new QwtPlotCurve();
    curve2->setPen(QPen(Qt::red, 0, Qt::SolidLine, Qt::RoundCap));
    curve2->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curve2->setTitle("Channel 2");
    curve2->attach(plot);

    d_marker_peak = new QwtPlotMarker();
    d_marker_peak->setValue( 0.0, 0.0 );
    d_marker_peak->setLineStyle( QwtPlotMarker::VLine);
    d_marker_peak->setLabelAlignment( Qt::AlignRight | Qt::AlignCenter );
    d_marker_peak->setLinePen( Qt::green, 0, Qt::DashDotLine );
    d_marker_peak->attach(plot);

    d_marker_peak_max = new QwtPlotMarker();
    d_marker_peak_max->setValue( 0.0, 0.0 );
    d_marker_peak_max->setLineStyle( QwtPlotMarker::VLine);
    d_marker_peak_max->setLabelAlignment( Qt::AlignRight | Qt::AlignTop);
    d_marker_peak_max->setLinePen( Qt::yellow, 0, Qt::DashDotDotLine );
    d_marker_peak_max->attach(plot);

    d_marker_peak_min = new QwtPlotMarker();
    d_marker_peak_min->setValue( 0.0, 0.0 );
    d_marker_peak_min->setLineStyle( QwtPlotMarker::VLine);
    d_marker_peak_min->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
    d_marker_peak_min->setLinePen( Qt::yellow, 0, Qt::DashDotDotLine );
    d_marker_peak_min->attach(plot);

    d_marker_threshold1 = new QwtPlotMarker();
    d_marker_threshold1->setValue( 0.0, PEAK_JUDGE );
    d_marker_threshold1->setLineStyle( QwtPlotMarker::HLine);
    d_marker_threshold1->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
    d_marker_threshold1->setLinePen( Qt::green, 0, Qt::DashDotLine );
    d_marker_threshold1->attach(plot);

    d_marker_threshold2 = new QwtPlotMarker();
    d_marker_threshold2->setValue( 0.0, -PEAK_JUDGE );
    d_marker_threshold2->setLineStyle( QwtPlotMarker::HLine);
    d_marker_threshold2->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
    d_marker_threshold2->setLinePen( Qt::green, 0, Qt::DashDotLine );
    d_marker_threshold2->attach(plot);


    x = 0;
    max=0;
    min=0;
    scale = 1.0;
    max_i = 0;
    min_i = 0;
    max_show_flag = true;
    min_show_flag = true;
//    peak_show_flag = false;

    this->hide();
}

RecWaveForm::~RecWaveForm()
{
}

void RecWaveForm::working(CURRENT_KEY_VALUE *val, QString str)
{
    if (val == NULL) {
        return;
    }

    key_val = val;

    if(key_val->grade.val0 != menu_index){
        return;
    }

    emit show_indicator(true);
    plot->setTitle(str + tr("\n按OK键寻找峰值"));

    FileTools *filetools = new FileTools(str,FileTools::Read);      //开一个线程，为了不影响数据接口性能
    if(str.contains("HFCT")){           //读取滤波器信息
        QString filter_info = filetools->get_filter_info();
        if(!filter_info.isEmpty()){
             plot->setTitle(str + tr("\n%1, 按OK键寻找峰值").arg(filter_info));
        }
    }

    connect(filetools,SIGNAL(readFinished(VectorList,MODE)),this,SLOT(start_work(VectorList,MODE)) );
    QThreadPool::globalInstance()->start(filetools);
}

void RecWaveForm::working(CURRENT_KEY_VALUE *val,VectorList buf, MODE mod)
{
    if (val == NULL) {
        return;
    }

    key_val = val;

    if(key_val->grade.val0 != menu_index){
        return;
    }
    mode = mod;
    plot->setTitle(tr("按OK键寻找峰值"));

    start_work(buf,mod);
}

void RecWaveForm::start_work(VectorList buf, MODE mode)
{
    setData(buf, mode);
    set_canvas();
    fresh();

    max_show_flag = true;
    min_show_flag = true;
    emit show_indicator(false);
    this->show();
}

void RecWaveForm::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if(key_val->grade.val5 == 0 ){
        return;
    }
    if(this->isHidden()){
        return;
    }

    switch (key_code) {
    case KEY_OK:
        d_marker_threshold1->show();
        d_marker_threshold2->show();
        d_marker_peak_max->show();
        d_marker_peak_min->show();
        if(max_show_flag){
            show_max();
        }
        else if(min_show_flag){
            show_min();
        }
        else if(peak_show_flag){
            find_peaks();
        }
        else{
            show_max();
        }
        break;
    case KEY_CANCEL:
        this->hide();
        key_val->grade.val5 = 0;
        if(key_val->grade.val0 != 6){
            key_val->grade.val1 = 0;
            key_val->grade.val2 = 0;
            emit fresh_parent();
        }
        if(mode == Double_Channel){
            plot->legend()->deleteLater();
        }
        break;
    case KEY_UP:
        scale = scale * 1.2;
        break;
    case KEY_DOWN:
        scale = scale / 1.2;
        break;
    case KEY_LEFT:
        if(x<100){
            x = 0;
        }
        else{
            x = x -100;
        }
        break;
    case KEY_RIGHT:
        if(x<wave1.length()-400){
            x = x +100;
        }
        else{
            x = wave1.length() - 300;
        }
        break;
    default:
        break;
    }
    fresh();
}

void RecWaveForm::setData(VectorList buf, MODE mod)
{
    mode = mod;
    QPointF p;
    curve2->detach();
    wave1.clear();
    wave2.clear();
    max = Common::physical_value(buf.at(0), mode);
    min = max;
    max_i = 0;
    min_i = 0;
    double v_real = 0, temp = 0;
    int length = buf.length();
    if( mode == Double_Channel){
        length = buf.length() / 2;
        curve2->attach(plot);
        plot->insertLegend( new QwtLegend(),  QwtPlot::RightLegend, -2 );
    }
    for (int i = 0; i < length; ++i) {
        switch (mode) {
        case TEV1:
        case TEV1_CONTINUOUS:
        case TEV2:
        case TEV2_CONTINUOUS:
        case HFCT1:
        case HFCT1_CONTINUOUS:
        case HFCT2:
        case HFCT2_CONTINUOUS:
        case UHF1:
        case UHF1_CONTINUOUS:
        case UHF2:
        case UHF2_CONTINUOUS:
            v_real = Common::physical_value(buf.at(i), mode);
            p = QPointF(i*0.01, v_real);
            wave1.append(p);
            break;
        case AA1:
        case AA2:
        case AE1:
        case AE2:
            v_real = Common::physical_value(buf.at(i), mode);
            p = QPointF(i/400.0, v_real);
            wave1.append(p);
            break;
        case AA1_ENVELOPE:
        case AA2_ENVELOPE:
        case AE1_ENVELOPE:
        case AE2_ENVELOPE:
            v_real = Common::physical_value(buf.at(i), mode);
            p = QPointF(i/40.0, v_real);
            wave1.append(p);
            break;
        case Double_Channel:
            v_real = Common::physical_value(buf.at(i * 2), (MODE)sqlcfg->get_para()->menu_h1);
            p = QPointF(i*0.01,v_real);
            wave1.append(p);
            temp = Common::physical_value(buf.at(i * 2 + 1), (MODE)sqlcfg->get_para()->menu_h2);
            p = QPointF(i*0.01,temp);
            wave2.append(p);
            v_real = MAX(v_real,temp);
            break;
        default:
            break;
        }

        if(v_real>max){
            max = v_real;
            max_i = i;
        }
        else if(v_real<min){
            min = v_real;
            min_i = i;
        }

    }
}

void RecWaveForm::set_canvas()
{
    if(min > d_marker_threshold2->yValue() && max < d_marker_threshold1->yValue()){
        peak_show_flag = false;
    }
    else{
        peak_show_flag = true;
    }
    if(min > -SHOW_FACTOR){
        min = -SHOW_FACTOR;
    }
    if(max < SHOW_FACTOR ){
        max = SHOW_FACTOR;
    }

    d_marker_peak_max->setValue(wave1.at(max_i));
    QString label;
    label.sprintf( "max %.3g", wave1.at(max_i).y() );
    QwtText text( label );
    text.setColor( Qt::darkYellow );
    d_marker_peak_max->setLabel( text );

    d_marker_peak_min->setValue(wave1.at(min_i));
    label.sprintf( "min %.3g", wave1.at(min_i).y() );
    text.setText(label);
    d_marker_peak_min->setLabel( text );

    plot->setAxisScale(QwtPlot::yLeft, min, max);
    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setSpacing(10);
    if(mode == AA1 || mode == AA2 || mode == AE1 || mode == AE2){
        plot->axisWidget(QwtPlot::xBottom)->setTitle("ms");
        plot->axisWidget(QwtPlot::yLeft)->setTitle("V u");
    }
    else{
        plot->axisWidget(QwtPlot::xBottom)->setTitle("us");
        plot->axisWidget(QwtPlot::yLeft)->setTitle("V m");
        d_marker_threshold1->setValue(0.0, Common::physical_threshold(mode));
        d_marker_threshold2->setValue(0.0, -Common::physical_threshold(mode));
    }

    d_marker_peak->hide();    
    d_marker_threshold1->hide();    
    d_marker_threshold2->hide();
    d_marker_peak_max->hide();
    d_marker_peak_min->hide();

    x = 0;   //开始显示最左边数据
    scale = 1.0;    //纵坐标拉伸因子为1
}

void RecWaveForm::find_peaks()
{
    int j, j1, j2;
    bool a,b;

    for (int i = 0; i < wave1.length(); ++i) {
        j = i + x + 101;
        j1 = j - 1;
        j2 = j + 1;
        if( j2 > wave1.length()){
            j -= wave1.length();        //循环搜索
            j1 -= wave1.length();
            j2 -= wave1.length();
            if(j1 < 0 || j2 < 0){
                continue;
            }
        }
        a = (wave1.at(j).y() > d_marker_threshold1->yValue())
                && (wave1.at(j).y() > wave1.at(j1).y())
                && (wave1.at(j).y() > wave1.at(j2).y());
        b = (wave1.at(j).y() < d_marker_threshold2->yValue())
                && (wave1.at(j).y() < wave1.at(j1).y())
                && (wave1.at(j).y() < wave1.at(j2).y());

        if( a || b){
            x = j-100;
            if(x < 0){
                x = 0;
            }
            d_marker_peak->setValue(wave1.at(j));
            QString label;
            label.sprintf( "peak %.3g", wave1.at(j).y() );
            QwtText text( label );
            text.setColor( Qt::darkGreen );
            d_marker_peak->setLabel( text );
            qDebug()<<"find peaks!"<<wave1.at(j);
            d_marker_peak->show();
            break;
        }
    }
}

void RecWaveForm::show_max()
{
    x = max_i-100;
    if(x < 0){
        x = 0;
    }
    max_show_flag = false;
    if(!peak_show_flag){
        min_show_flag = true;
    }
}

void RecWaveForm::show_min()
{
    x = min_i-100;
    if(x < 0){
        x = 0;
    }
    min_show_flag = false;
    if(!peak_show_flag){
        max_show_flag = true;
    }
}

void RecWaveForm::setScroll(int value)
{
    if(wave1.length()-value > 300){
        QVector<QPointF> w;
        w = wave1.mid(value,300);
        curve1->setSamples(w);

        if(mode == Double_Channel){
            curve2->setSamples(wave2.mid(value,300));
        }

        plot->setAxisScale(QwtPlot::xBottom, w.first().x(), w.last().x());
        plot->replot();
    }
}

void RecWaveForm::fresh()
{
    this->setScroll(x);
    plot->setAxisScale(QwtPlot::yLeft, min * scale , max * scale);

    plot->replot();
}

double RecWaveForm::get_peak_freq(int i)
{

}









