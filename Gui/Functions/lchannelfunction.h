#ifndef LCHANNELFUNCTION_H
#define LCHANNELFUNCTION_H

#include "basechannlfunction.h"

class LChannelFunction : public BaseChannlFunction
{
    Q_OBJECT
public:
    explicit LChannelFunction(G_PARA *data, MODE mode, QObject *parent = nullptr);

    virtual void channel_start();               //通道开始工作接收数据

    void change_vol(int d);             //改变音量
    void reset_vol();                   //重置音量

    void save_rdb_data();               //保存rdb数据

    void clear_100ms();

    QVector<int> spectra_100ms();

    int v_50Hz();
    int v_100Hz();


signals:

public slots:
    void compute_100ms();               //定时器触发
    void compute_1000ms();              //定时器触发
    void read_envelope_data(VectorList list);     //处理128个点的AE数据（3.2ms的数据）

private:
    QVector<int> envelope_datalist;
    QVector<int> envelope_timelist;

    float temp_db;          //db显示缓冲区
    float last_1000ms_max_val;      //前1秒内最大值

    QVector<int> ae_fftlist;
    FFT *fft;
    int _v_50Hz,_v_100Hz;

    void compute_db_100ms();
    void compute_pulse_100ms();

//    QVector<QPoint> calc_pulse_list();          //根据给出的序列和阈值计算脉冲序列


};

#endif // LCHANNELFUNCTION_H
