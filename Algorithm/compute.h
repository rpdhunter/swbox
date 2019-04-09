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

//     static QPoint trans_data(int x, int y, MODE mode);     //将原始脉冲转换成可显示的DB值
     static double compute_amp(qint32 code_val, MODE mode);          //计算幅值
     static QVector<PC_DATA> compute_pc_1ms(QVector<int> list, int x_origin, double gain, int threshold);        //计算1ms数据的HFCT数据
     static QPoint find_max_point(QVector<int> list);           //寻找峰值点
     static QVector<int> find_max_peak(QVector<int> list);      //寻找包含最值的波形数据
     static QVector<int> find_max_peak(QVector<int> list, QPoint max_Point);      //寻找包含最值的波形数据

     static PC_DATA compute_pc_1node(QVector<int> list, int x_origin, double gain);           //计算一个脉冲节点的HFCT数据
     static double simpson(QVector<int> list);
     static double triangle(double d1, double d2);
     static void compute_f_value(QVector<int> list, FFT *fft, int &v_50Hz, int &v_100Hz, MODE mod);
     static QPoint max(QVector<int> list);      //返回最大值点
     static QPoint min(QVector<int> list);      //返回最小值点
     static QPoint max_abs(QVector<int> list);  //返回绝对值最大点
     static QVector<int> sim_sin(int amp, int f, int n);        //生成模拟正弦波,频率为f MHz,幅值为amp, 点数是n
     static QVector<int> sim_pulse(int amp, int n);        //生成模拟脉冲,幅值为amp, 点数是n
     static double l_channel_modify(double val);            //低频通道系数修正
     static double interpolation(double x, double x1, double x2, double f1, double f2); //插值公式
     static float phase_error(float phase, QList<float> list);      //相位误差
signals:

public slots:
};

#endif // COMPUTE_H
