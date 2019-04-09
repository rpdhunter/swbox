#include "flychart.h"

FlyChart::FlyChart(QObject *parent) : BaseChart(parent)
{
}

void FlyChart::chart_init(QWidget *parent, MODE mode)
{
    BaseChart::chart_init(parent, mode);

    plot = new QwtPlot(parent);
    plot->move(10,35);
    plot->resize(200, 140);
    plot->setStyleSheet("background:transparent;color:gray;");

    d_fly = new QwtPlotSpectroCurve;

    plot->setAxisScale(QwtPlot::xBottom, 0, 200);
    plot->setAxisScale(QwtPlot::yLeft, 0, max_value);
    /* remove gap */
    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);
    d_fly->attach(plot);

    timer_fly = new QTimer(this);
    timer_fly->setInterval(20000);
    connect(timer_fly, SIGNAL(timeout()), this, SLOT(reset_data()));   //每20秒刷新一次数据状态
    timer_fly->start();
}

void FlyChart::add_data(QVector<QwtPoint3D> point_list)
{
    fly_samples.append(point_list);
    d_fly->setSamples(fly_samples);
    plot->replot();
}

void FlyChart::add_data(QVector<QPoint> point_list)
{
    QVector<QwtPoint3D> newlist;
    foreach (QPoint P, point_list) {
        newlist.append(QwtPoint3D(P.x()*20/180, P.y(), 1));
    }
    add_data(newlist);
}

void FlyChart::reset_data()
{
    fly_samples.clear();
    plot->replot();
}
