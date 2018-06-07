#ifndef COMPUTE_H
#define COMPUTE_H

#include <QObject>
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "fft.h"

class Compute : public QObject
{
    Q_OBJECT
public:
    explicit Compute(QObject *parent = nullptr);

     static QPoint trans_data(int x, int y, MODE mode);     //将原始脉冲转换成可显示的DB值

     static QVector<PC_DATA> compute_pc_1ms(QVector<int> list, int x_origin, double gain);        //计算1ms数据的HFCT数据
     static PC_DATA compute_pc_1node(QVector<int> list, int x_origin, double gain);           //计算一个脉冲节点的HFCT数据
     static double simpson(QVector<int> list);
     static double triangle(double d1, double d2);
     static void compute_f_value(QVector<int> list, FFT *fft, int &v_50Hz, int &v_100Hz);
signals:

public slots:
};

#endif // COMPUTE_H
