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
    d_marker_peak->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
    d_marker_peak->setLinePen( Qt::green, 0, Qt::DashDotLine );
    d_marker_peak->attach(plot);


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
    QThreadPool::globalInstance()->start(filetools);
    connect(filetools,SIGNAL(readFinished(VectorList,MODE)),this,SLOT(start_work(VectorList,MODE)) );
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
    emit show_indicator(false);
    this->show();
}

void RecWaveForm::trans_key(quint8 key_code)
{
    //    qDebug()<<"BBBBB"<<key_val->grade.val5;
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
        find_peaks();
        break;
    case KEY_CANCEL:
        this->hide();
        key_val->grade.val5 = 0;
        if(key_val->grade.val0 != 5){
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
    max=0;
    min=0;
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
            v_real = sqlcfg->get_para()->tev1_sql.gain * TEV_FACTOR * buf.at(i);
            p = QPointF(i*0.01, v_real);
            wave1.append(p);
            break;
        case TEV2:
        case TEV2_CONTINUOUS:
            v_real = sqlcfg->get_para()->tev2_sql.gain * TEV_FACTOR * buf.at(i);
            p = QPointF(i*0.01, v_real);
            wave1.append(p);
            break;
        case HFCT1:
        case HFCT1_CONTINUOUS:
            v_real = sqlcfg->get_para()->hfct1_sql.gain * TEV_FACTOR * buf.at(i);
            p = QPointF(i*0.01, v_real);
            wave1.append(p);
            break;
        case HFCT2:
        case HFCT2_CONTINUOUS:
            v_real = sqlcfg->get_para()->hfct2_sql.gain * TEV_FACTOR * buf.at(i);
            p = QPointF(i*0.01, v_real);
            wave1.append(p);
            break;
        case AA1:
            v_real = (buf.at(i) * 4 ) * sqlcfg->get_para()->aa1_sql.gain * AA_FACTOR;
            p = QPointF(i/320.0, v_real);
            wave1.append(p);
            break;
        case AA2:
            v_real = (buf.at(i) * 4 ) * sqlcfg->get_para()->aa2_sql.gain * AA_FACTOR;
            p = QPointF(i/320.0, v_real);
            wave1.append(p);
            break;
        case Double_Channel:
            v_real = buf.at(i * 2) * sqlcfg->get_para()->tev1_sql.gain * TEV_FACTOR;
            p = QPointF(i*0.01,v_real);
            wave1.append(p);
            temp = buf.at(i * 2 + 1) * sqlcfg->get_para()->tev2_sql.gain * TEV_FACTOR;
            p = QPointF(i*0.01,temp);
            wave2.append(p);
//            curve2->attach(plot);
            v_real = MAX(v_real,temp);
            break;
        default:
            break;
        }

        if(v_real>max){
            max = v_real;
        }
        else if(v_real<min){
            min = v_real;
        }

    }

    set_canvas();
//    plot->setTitle(tr("\n按OK键寻找峰值"));
    fresh();
}

void RecWaveForm::set_canvas()
{
    if(min > -SHOW_FACTOR){
        min = -SHOW_FACTOR;
    }
    if(max < SHOW_FACTOR ){
        max = SHOW_FACTOR;
    }

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

    x = 0;   //开始显示最左边数据
    scale = 1.0;    //纵坐标拉伸因子为1
}

void RecWaveForm::find_peaks()
{
    d_marker_threshold1->show();
    d_marker_threshold2->show();

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
            label.sprintf( " peak = %.3g", wave1.at(j).y() );
            QwtText text( label );
            text.setColor( Qt::darkGreen );
            d_marker_peak->setLabel( text );
            qDebug()<<"find peaks!"<<wave1.at(j);
            d_marker_peak->show();
            break;

        }
    }
    fresh();
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

        //        qDebug()<<"w.first().x() = "<<w.first().x()<<"\tw.last().x() = "<<w.last().x();

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









