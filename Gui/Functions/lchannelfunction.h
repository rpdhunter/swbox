#ifndef LCHANNELFUNCTION_H
#define LCHANNELFUNCTION_H

#include "basechannlfunction.h"
//#include "IO/Com/modbus/uartmodbus.h"

class LChannelFunction : public BaseChannlFunction
{
    Q_OBJECT
public:
    explicit LChannelFunction(G_PARA *data, MODE mode, QObject *parent = nullptr);

    void channel_start();               //通道开始工作接收数据

    void change_vol(int d);             //改变音量
    void reset_vol();                   //重置音量

    void save_rdb_data();               //保存rdb数据
    void save_modbus_data();            //发送modbus数据

    void clear_100ms();
    void clear_1000ms();

    QVector<int> spectra_100ms();

    float v_50Hz() const;
    float v_100Hz() const;
    float v_effective() const;
    float v_peak() const;

signals:

public slots:
    void compute_100ms();               //定时器触发
    void compute_1000ms();              //定时器触发
    void read_envelope_data(VectorList list);     //处理128个点的AE数据（3.2ms的数据）

private:
    QVector<int> envelope_datalist;
    QVector<int> envelope_timelist;

    float temp_db;                  //db显示缓冲区
    float last_1000ms_max_val;      //前1秒内最大值

    QVector<float> ae_fftlist;
    FFT *fft;
    QVector<float> _v_50Hz, _v_100Hz, _v_effective, _v_peak;

    void compute_db_100ms();
    void compute_pulse_100ms();

//    UartModbus *uartmodbus;
    char *buf;

//    QVector<QPoint> calc_pulse_list();          //根据给出的序列和阈值计算脉冲序列


};

#endif // LCHANNELFUNCTION_H
