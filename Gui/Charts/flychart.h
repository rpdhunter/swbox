#ifndef FLYCHART_H
#define FLYCHART_H

#include "basechart.h"
#include <QTimer>

class FlyChart : public BaseChart
{
    Q_OBJECT
public:
    explicit FlyChart(QObject *parent = nullptr);

    void chart_init(QWidget *parent, MODE mode);  //初始化
    void add_data(QVector<QwtPoint3D> point_list);      //增加数据点 QVector<QPoint(相位，幅值)>
    void add_data(QVector<QPoint> point_list);

//    void save_data();               //保存数据到文件

signals:

public slots:
    void reset_data();              //数据重置

private:
    QwtPlotSpectroCurve *d_fly;        //飞行曲线
    QVector<QwtPoint3D> fly_samples;   //飞行数据
    int fly_number;                    //飞行周期计数（目前20个周期重置）
    QTimer *timer_fly;
};

#endif // FLYCHART_H
