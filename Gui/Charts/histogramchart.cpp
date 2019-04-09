#include "histogramchart.h"

HistogramChart::HistogramChart(QObject *parent) : BaseChart(parent)
{    
}

void HistogramChart::chart_init(QWidget *parent, MODE mode)
{
    BaseChart::chart_init(parent, mode);

    plot = new QwtPlot(parent);
    plot->move(10,35);
    plot->resize(200, 140);
    plot->setStyleSheet("background:transparent;color:gray;");

    d_histogram = new QwtPlotHistogram;

    if(mode == AE1 || mode == AE2){         //AE模式下是特征指数图
        plot->setAxisScale(QwtPlot::xBottom, 0, 6);
        plot->setAxisScale(QwtPlot::yLeft, 0, 100);
    }
    else{                                   //其他模式(现在仅有TEV模式),为脉冲强度计数
        plot->setAxisScale(QwtPlot::xBottom, min_value, max_value);
        plot->setAxisScale(QwtPlot::yLeft, 0, 100);
    }

    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::xBottom)->setTitle("dB");
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);

    //    plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );  //对数坐标
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->plotLayout()->setAlignCanvasToScales(true);

    d_histogram->setStyle( QwtPlotHistogram::Columns );
    d_histogram->setBrush(Qt::green);
    d_histogram->setPen(Qt::green);
    d_histogram->attach(plot);

    reset_data();

}

void HistogramChart::add_data(QVector<QPoint> point_list)
{
//    qDebug()<<point_list;
    if(mode == AE1 || mode == AE2){         //AE模式下是特征指数图(注意,此模式下送入的x坐标已经转化为全相位,代表0-100ms)
        int time;
        for (int i = 1; i < point_list.count(); ++i) {
//            time = point_list.at(i).x() - point_list.at(i-1).x();       //时间差(范围0-9 999 999)
//            if(time < 0){
//                time += 10000000;
//            }
//            if(sqlcfg->get_para()->freq_val == 50){         //这里转化为全相位差(范围0-1800)
//                time = time * 360 /2000000;                 //(范围0-1800)
//            }
//            else if(sqlcfg->get_para()->freq_val == 60){
//                time = time * 360 /1666667;                 //(范围0-2160)
//            }
            time = point_list.at(i).x() - point_list.at(i-1).x();       //时间差(范围0-9 999 999)
            if(sqlcfg->get_para()->freq_val == 50 && time < 0){         //这里转化为全相位差(范围0-1800)
                time += 1800;                   //(范围0-1800)
            }
            else if(sqlcfg->get_para()->freq_val == 60 && time < 0){
                time += 2160;                   //(范围0-2160)
            }
            time /= 18;                                     //(范围0-100)
            if(time < 60 && time >= 0){                     //仅显示前60
                histogram_data[time].value += 1;
            }
        }
    }
    else{                                   //其他模式(现在仅有TEV模式),为脉冲强度计数
        foreach (QPoint p, point_list) {
            if(p.y() <= max_value && p.y() >= min_value){
                histogram_data[p.y() - min_value].value += 1;
            }
        }
    }

    d_histogram->setData(new QwtIntervalSeriesData( histogram_data ));
    plot->replot();
}

void HistogramChart::reset_data()
{
    histogram_data.clear();

    if(mode == AE1 || mode == AE2){         //AE模式下是特征指数图
        for(int j=0;j<60;j++){
            QwtInterval interval( 0.1 * j, 0.1 * (j + 1));
            interval.setBorderFlags( QwtInterval::ExcludeMaximum );
            histogram_data.append( QwtIntervalSample( 0, interval ) );
        }
    }
    else{                                   //其他模式(现在仅有TEV模式),为脉冲强度计数
        for(int j=0;j<121;j++){
            QwtInterval interval( j - 60.0 , j - 59.0 );
            interval.setBorderFlags( QwtInterval::ExcludeMaximum );
            histogram_data.append( QwtIntervalSample( 0, interval ) );
        }
    }

    d_histogram->setData(new QwtIntervalSeriesData( histogram_data ));
    plot->replot();
}
