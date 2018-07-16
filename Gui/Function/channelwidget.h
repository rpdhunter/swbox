#ifndef CHANNELWIDGET_H
#define CHANNELWIDGET_H

#include <QFrame>
#include <QVector>
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Other/logtools.h"
#include "IO/Com/rdb/rdb.h"

#include "../Common/barchart.h"
#include "../Common/prpsscene.h"
#include "../Common/recwaveform.h"
#include "../Common/common.h"
#include "../Common/compute.h"
#include "../Common/fft.h"
#include "../Common/basewidget.h"

class QTimer;
class QwtPlotSpectroCurve;
class QwtPlotHistogram;
class QwtPlotCurve;
class QGraphicsView;

class ChannelWidget : public BaseWidget
{
    Q_OBJECT
public:
    explicit ChannelWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = nullptr);

    void change_log_dir();      //改变asset目录
    virtual void save_channel();        //保存通道数据

protected slots:
    virtual void showWaveData(VectorList buf, MODE mod);
    virtual void fresh_1000ms();
    virtual void fresh_100ms();
    virtual void fresh_1ms();
    virtual void trans_key(quint8 key_code);

private slots:
    void add_token();
    void close_rec();       //关闭录波系统
    void add_ae_data();     //处理256个点的AE数据（1/160秒的数据）

signals:
//    void fresh_parent();
//    void send_key(quint8);
//    void show_indicator(bool);      //显示菊花

    void startRecWave(MODE, int);
    void send_log_data(double val, int pulse, double degree, int qc);    //发送日志数据
    void send_PRPD_data(QVector<QwtPoint3D>);
    void beep(int index, int red_alert);        //蜂鸣器报警(参数：通道，严重程度(0,1,2))


protected:
//    virtual void do_key_up_down(int d) = 0;
//    virtual void do_key_left_right(int d) = 0;
    void do_key_ok();
    void do_key_cancel();
    virtual void fresh_setting() = 0;

    //    void chart_ini();
    void maxReset(QLabel *label);        //最大值清零
    void do_Spectra_compute();

    //常规变量
    G_PARA *data;
    G_RECV_PARA_SHORT *short_data;
    SQL_PARA sql_para;
    MODE mode;
    int menu_index;     //位于主菜单的位置索引
    LogTools *logtools;
    FFT *fft;


    G_RECV_PARA_ENVELOPE *ae_pulse_data;
    QVector<int> ae_datalist;
    QVector<int> ae_timelist;
    //定时器
    QTimer *timer_1ms, *timer_100ms, *timer_1000ms, *timer_freeze, *timer_rec_close_delay;

    //基本数据
    int db;
    int max_db;
    int db_last1;
    QVector<int> pulse_cnt_list;
    quint32 group_num;   //用于判别PRPD图数据有效性的组号(0-3变化)
//    void calc_tev_value (double &tev_db, int &pulse_cnt_show, double &degree, int &sug_central_offset, int &sug_noise_offset);
    QVector<QPoint> pulse_100ms;      //200ms的脉冲数据,分析的基准
    QVector<double> amp_1000ms;         //1s内的平均幅值序列（pulse_100ms每次清零时保存一个平均幅值，1s的时候再算总平均幅值）
    uint token;

    //时序图
    QwtPlot *plot_Barchart;
    BarChart *d_BarChart;              //棒状图

    //PRPD
    QwtPlot *plot_PRPD;                 //PRPD图
    QwtPlotSpectroCurve *d_PRPD;        //PRPD曲线
    QVector<QwtPoint3D> prpd_samples;   //PRPD数据
    int map[360][121];                  //PRPD存储中介(数据点图)
    QVector<QwtPoint3D> points_PRPD;//PRPD曲线数据(qwt)
    QMap<MyKey,int> map_PRPD;       //PRPD数据(Qt,points_PRPD的数据源,便与检索)

    //PRPS
    QGraphicsView *plot_PRPS;       //PRPS图
    PRPSScene *scene;

    //柱状图
    QwtPlot *plot_Histogram;
    QwtPlotHistogram *d_histogram;   //Histogram图
    QVector<QwtIntervalSample> histogram_data;

    //频谱图
    QwtPlot *plot_Spectra;
    QwtPlotHistogram *d_Spectra;
    QVector<QwtIntervalSample> Spectra_data;
    int Spectra_map[60];          //Spectra存储中介(数据点图)

    //t-f图
    QwtPlot *plot_TF;
    QwtPlotSpectroCurve *d_TF;    //TF曲线(qwt)
    QVector<QwtPoint3D> points_TF;//TF曲线数据(qwt)
    QMap<MyKey,int> map_TF;       //TF数据(Qt,points_TF的数据源,便与检索)

    //录波
    bool manual;        //手动录波标志
    RecWaveForm *recWaveForm;
};

#endif // CHANNELWIDGET_H
