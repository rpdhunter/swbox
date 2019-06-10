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
     static void compute_f_value(QVector<float> list, FFT *fft, float &v_50Hz, float &v_100Hz, float &effective, MODE mod);
     static QVector<float> interpolation(QVector<int> input, int in_num, int out_num);        //in_num点插成out_num点
//     static QPoint max(QVector<int> list);      //返回最大值点
//     static QPoint min(QVector<int> list);      //返回最小值点
     static QPoint max_abs(QVector<int> list);  //返回绝对值最大点
     static QVector<int> sim_sin(int amp, int f, int n);        //生成模拟正弦波,频率为f MHz,幅值为amp, 点数是n
     static QVector<int> sim_pulse(int amp, int n);        //生成模拟脉冲,幅值为amp, 点数是n
     static double l_channel_modify(double val);            //低频通道系数修正
     static double l_channel_modify_inverse(double val);            //低频通道系数修正(逆)
     static double interpolation2(double x, double x1, double x2, double f1, double f2); //两点线性插值公式
     static float phase_error(float phase, QList<float> list);      //相位误差

     static int max_at(QVector<int> list);              //找到最大值的位置，返回序号
     static float max_at(QVector<float> list);          //找到最大值的位置，返回序号
     static double max_at(QVector<double> list);        //找到最大值的位置，返回序号

     static int min_at(QVector<int> list);              //找到最小值的位置，返回序号
     static float min_at(QVector<float> list);          //找到最小值的位置，返回序号
     static double min_at(QVector<double> list);        //找到最小值的位置，返回序号

     static int max(QVector<int> list);                 //返回最大值
     static float max(QVector<float> list);             //返回最大值
     static double max(QVector<double> list);           //返回最大值

     static int min(QVector<int> list);                 //返回最小值
     static float min(QVector<float> list);             //返回最小值
     static double min(QVector<double> list);           //返回最小值

     static int sum(QVector<int> list);                 //和
     static float sum(QVector<float> list);
     static double sum(QVector<double> list);

     static int avrage(QVector<int> list);              //平均值
     static float avrage(QVector<float> list);
     static double avrage(QVector<double> list);

     static float square_sum_root(QVector<float> list); //方和根

     static QVector<QPoint> calc_pulse_list(QVector<int> datalist, QVector<int> timelist, int threshold, MODE mode, int max_num = 0);          //根据给出的序列和阈值计算脉冲序列

signals:

public slots:
};

#endif // COMPUTE_H
