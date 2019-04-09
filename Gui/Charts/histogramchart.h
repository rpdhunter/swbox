#ifndef HISTOGRAMCHART_H
#define HISTOGRAMCHART_H

#include "basechart.h"

class HistogramChart : public BaseChart
{
    Q_OBJECT
public:
    explicit HistogramChart(QObject *parent = nullptr);

    void chart_init(QWidget *parent, MODE mode);  //初始化
    void add_data(QVector<QPoint> point_list);      //增加数据点 QVector<QPoint(相位，幅值)>
    void reset_data();              //数据重置
//    void save_data();               //保存数据到文件

signals:

public slots:

private:
    QwtPlotHistogram *d_histogram;   //Histogram图
    QVector<QwtIntervalSample> histogram_data;
};

#endif // HISTOGRAMCHART_H
