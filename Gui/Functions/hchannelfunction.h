#ifndef HCHANNELFUNCTION_H
#define HCHANNELFUNCTION_H

#include "basechannlfunction.h"
//#include <QPoint>

class HChannelFunction : public BaseChannlFunction
{
    Q_OBJECT
public:
    explicit HChannelFunction(G_PARA *data, MODE mode, QObject *parent = nullptr);

//    void channel_init();                //通道数据初始化
    void channel_start();               //通道开始工作接收数据

//    void toggle_test_status();          //启动-停止测试
//    void change_chart(int d);           //改变图形显示
    void change_threshold(int d);       //改变脉冲阈值(需FPGA操作)
    void reset_threshold();             //重置脉冲阈值
    void toggle_auto_rec();             //切换自动录波状态(需FPGA操作)
    void reset_auto_rec();              //重置自动录波状态
    void click_rec_single();            //点击单次录波
//    void change_rec_cons_time(int d);   //改变连续录波时长
//    void click_rec_cons();              //点击连续录波
//    void toggle_units();                //切换计量单位
    void change_HF(int d);              //改变高通滤波器
    void change_LF(int d);              //改变低通滤波器
//    void click_reset();                 //点击测量值重置

//    void toggle_test_mode();            //切换检测模式
    void change_gain(int d);            //改变增益
//    void change_red_alarm(int d);       //改变红色报警阈值
//    void change_yellow_alarm(int d);    //改变黄色报警阈值
//    void toggle_buzzer();               //切换蜂鸣器报警状态
//    void change_pulse_time(int d);      //改变脉冲计数时长
//    void toggle_mode_recognition();     //切换模式识别状态

    void save_rdb_data();               //保存rdb数据




    QVector<PC_DATA> pclist_100ms();

    void clear_100ms();

    void clear_1000ms();

    QVector<int> spectra_100ms();


public slots:
    void read_short_data();
    void compute_100ms();               //定时器触发
    void compute_1000ms();              //定时器触发

protected slots:

private slots:
    void add_token();           //令牌增加的速率需要再评估

private:
    void compute_db_1000ms();
    void compute_pulse_1000ms();
    uint token;                             //限速令牌
    quint32 short_time;                     //用于判别短录波数据有效性

    QVector<double> amp_1000ms;         //1s内的平均幅值序列（pulse_100ms每次清零时保存一个平均幅值，1s的时候再算总平均幅值）

    QVector<PC_DATA> _pclist_100ms;      //100ms的脉冲数据,HFCT分析的基准

    double code_to_db(qint32 code_val);     //码值转成DB值

    QVector<int> _spectra_100ms;          //频谱数据
    qint32 max_100ms;                       //100ms内的最大值(码值)


};

#endif // HCHANNELFUNCTION_H
