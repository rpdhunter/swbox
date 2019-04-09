#ifndef TFCHART_H
#define TFCHART_H

#include "basechart.h"

class TFChart : public BaseChart
{
    Q_OBJECT
public:
    explicit TFChart(QObject *parent = nullptr);

    void chart_init(QWidget *parent, MODE mode);               //初始化
    void add_data(QVector<PC_DATA> pc_list);      //增加数据点 QVector<QPoint(相位，幅值)>
    void reset_data();              //数据重置

signals:

public slots:

private:
    QwtPlotSpectroCurve *d_TF;    //TF曲线(qwt)
    QVector<QwtPoint3D> points_TF;//TF曲线数据(qwt)
    QMap<MyKey,int> map_TF;       //TF数据(Qt,points_TF的数据源,便与检索)
};

#endif // TFCHART_H
