#ifndef BASECHANNLFUNCTION_H
#define BASECHANNLFUNCTION_H

#include <QObject>
#include <QTimer>
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "../Common/common.h"
#include "Algorithm/compute.h"
#include "Algorithm/fir.h"
#include "IO/Com/rdb/point_table.h"

/***********************************************
 *  从Ui层分离出来的功能类,主要完成以下功能:
 *  1.从数据线程得到数据,并加工成可显示的数据
 *  2.完成Ui传来的一些指令
 *  3.Ui从此不再直接接触数据线程,也不能直接修改sql
 *  4.此类按照物理通道对接数据线程
 *  5.此类按照逻辑通道对接Ui
 * *********************************************/
class BaseChannlFunction : public QObject
{
    Q_OBJECT
public:
    explicit BaseChannlFunction(G_PARA *data, MODE mode, QObject *parent = nullptr);

    void channel_init();                //通道数据初始化
    virtual void channel_start();               //通道开始工作接收数据

    virtual void toggle_test_status();          //启动-停止测试
    virtual void change_chart(int d);           //改变图形显示
    virtual void change_threshold(int d);       //改变脉冲阈值
//    void toggle_auto_rec();             //切换自动录波状态
//    void click_rec_single();            //点击单次录波
    virtual void change_rec_cons_time(int d);   //改变连续录波时长
//    void click_rec_cons();              //点击连续录波
    virtual void toggle_units();                //切换计量单位
//    void change_HF(int d);              //改变高通滤波器
//    void change_LF(int d);              //改变低通滤波器
    virtual void click_reset();                 //点击测量值重置

    virtual void toggle_test_mode();            //切换检测模式
    virtual void change_gain(int d);            //改变增益
    virtual void change_red_alarm(int d);       //改变红色报警阈值
    virtual void change_yellow_alarm(int d);    //改变黄色报警阈值
    virtual void toggle_buzzer();               //切换蜂鸣器报警状态
    virtual void change_pulse_time(int d);      //改变脉冲计数时长
    virtual void toggle_mode_recognition();     //切换模式识别状态




    void set_current(int is_current);   //设置前台
    int is_current(){
        return _is_current;
    }


    void save_sql();                    //保存sql
    void cancel_sql();                  //放弃保存sql
    int val();                          //返回显示值
    int max_val();                      //返回最大值
    int pulse_cnt();                    //返回脉冲数
    float degree();                     //返回严重度
    CHANNEL_SQL *sql();                 //返回通道SQL参数
    QVector<QPoint> pulse_100ms();      //返回100ms脉冲序列

signals:
    void fresh_100ms();
    void fresh_1000ms();
    void fresh_gusty();             //突发刷新

public slots:

protected:
    G_PARA *data;
    MODE mode;
    int channel_num;        //通道编号

    SQL_PARA sql_para;
    CHANNEL_SQL *channel_sql;
    G_RECV_PARA_SHORT *short_data;          //高频短录波数据
    G_RECV_PARA_ENVELOPE *envelope_data;    //低频包络线数据

    Fir *fir;


    float db_val, pc_val, mv_val, show_val;
    int _max_val;

    int sug_zero_offset, sug_noise_offset;  //建议值
    QVector<int> pulse_cnt_list;
    QVector<QPoint> _pulse_100ms;        //100ms的脉冲数据,prpd,prps分析的基准
    int _pulse_cnt_show;          //脉冲数
    float _degree;                //严重度
    int _is_current;               //当前通道是否位于前台


    QTimer *timer_token, *timer_100ms, *timer_1000ms;


};

#endif // BASECHANNLFUNCTION_H
