#include "tfchart.h"

TFChart::TFChart(QObject *parent) : BaseChart(parent)
{
}

void TFChart::chart_init(QWidget *parent, MODE mode)
{
    BaseChart::chart_init(parent, mode);

    plot = new QwtPlot(parent);
    plot->move(10,35);
    plot->resize(200, 140);
    plot->setStyleSheet("background:transparent;color:gray;");

    plot->setAxisScale(QwtPlot::xBottom, 0, 360, 180);
    plot->setAxisScale(QwtPlot::yLeft, -max_value, max_value, max_value);

    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);

//    plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine );  //对数坐标
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->plotLayout()->setAlignCanvasToScales(true);

    QwtLinearColorMap *colorMap = new QwtLinearColorMap;
    colorMap->setColorInterval(Qt::blue,Qt::red);
    colorMap->addColorStop(0.3,Qt::green);
    colorMap->addColorStop(0.6,Qt::yellow);

    d_TF = new QwtPlotSpectroCurve;
    d_TF->setColorMap(colorMap);
    d_TF->setColorRange(QwtInterval(1,6));
    QwtScaleWidget *rightAxis = plot->axisWidget( QwtPlot::yRight );
    rightAxis->setColorBarEnabled( true );
    rightAxis->setColorMap(QwtInterval(1,6),colorMap);

    plot->setAxisScale( QwtPlot::yRight, 1, 6 );
    plot->enableAxis( QwtPlot::yRight );

    plot->plotLayout()->setAlignCanvasToScales( true );
    d_TF->attach(plot);
}

void TFChart::add_data(QVector<PC_DATA> pc_list)
{
    MyKey key_TF;
    double k;   //TF计算中使用的系数
    foreach (PC_DATA point, pc_list) {
        if(point.rise_time == 0){
            point.rise_time = 1;
        }
        k = point.fall_time / point.rise_time;
        key_TF = MyKey(point.rise_time , k * 1000 / (point.rise_time + point.fall_time) ) ;

        if( map_TF.contains(key_TF) ){
            map_TF[key_TF] = map_TF.value(key_TF) + 1;
        }
        else{
            map_TF.insert(key_TF,1);
        }

        points_TF.append(QwtPoint3D(key_TF.x, key_TF.y, qAbs(point.pc_value) / 40  ) );
    }

    d_TF->setSamples(points_TF);
    plot->setAxisScale(QwtPlot::yLeft, 1, 100, 100);
    plot->setAxisScale(QwtPlot::xBottom, 1, 100, 100);
    plot->replot();
}

void TFChart::reset_data()
{
    map_TF.clear();
    points_TF.clear();
}





