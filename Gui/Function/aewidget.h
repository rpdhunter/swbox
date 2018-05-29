#ifndef AEWIDGET_H
#define AEWIDGET_H

#include <QFrame>
#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Other/logtools.h"

#include "../Common/barchart.h"
#include "../Common/recwaveform.h"
#include "../Common/common.h"

namespace Ui {
class AEWidget;
}

class QTimer;
class FFT;

class AEWidget : public QFrame
{
    Q_OBJECT
public:
    explicit AEWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = 0);
    ~AEWidget();

public slots:
    void reload(int index);
    void trans_key(quint8 key_code);
    void showWaveData(VectorList buf, MODE mod);
    void change_log_dir();      //改变asset目录
    void PRPDReset();
    void save_channel();        //保存通道数据

signals:
    void send_key(quint8);
    void fresh_parent();
    void startRecWave(MODE, int);        //开始录播
    void ae_log_data(double,int,double);
    void show_indicator(bool);      //显示菊花
    void beep(int index, int red_alert);       //蜂鸣器报警
    void ae_PRPD_data(QVector<QwtPoint3D>);

private slots:
    void fresh(bool f); //刷新数据核
    void fresh_1000ms();
    void fresh_100ms();
    void add_ae_data();     //处理256个点的AE数据（1/160秒的数据）
    void fly_Reset();

private:
    Ui::AEWidget *ui;    
    CURRENT_KEY_VALUE *key_val;
    G_PARA *data;
    int menu_index;
    SQL_PARA sql_para;
    L_CHANNEL_SQL *aeultra_sql;
    MODE mode;

    int db;         //每秒的最大值，用于给图形传递参数
    double max_db;  //最大值
    double temp_db; //显示值缓冲区，用于减缓刷新

    QTimer *timer_100ms , *timer_1000ms, *timer_10000ms;
    RecWaveForm *recWaveForm;
    LogTools *logtools;
    bool isBusy;            //菊花状态
    G_RECV_PARA_AE *ae_pulse_data;

    QVector<int> ae_datalist;
    QVector<int> ae_timelist;
    QVector<int> ae_fftlist;
    FFT *fft;

    void fresh_setting();
    void maxReset();
//    void calc_aa_value (double * aa_val, double * aa_db, int * offset);
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void chart_ini();

    //时序图
    QwtPlot *plot_Barchart;
    BarChart *d_BarChart;              //棒状图

    //PRPD
    QwtPlot *plot_PRPD;                 //PRPD图
    QwtPlotSpectroCurve *d_PRPD;        //PRPD曲线
    QVector<QwtPoint3D> prpd_samples;   //PRPD数据
    int prpd_map[360][121];                  //PRPD存储中介(数据点图)
    QPoint transData(int x, int y);     //将原始脉冲转换成可显示的DB值


    //飞行图
    QwtPlot *plot_fly;                 //飞行图
    QwtPlotSpectroCurve *d_fly;        //飞行曲线
    QVector<QwtPoint3D> fly_samples;   //飞行数据
    int fly_number;                    //飞行周期计数（目前20个周期重置）


    //柱状图
    QwtPlot *plot_Histogram;
    QwtPlotHistogram *d_histogram;   //Histogram图
    QVector<QwtIntervalSample> histogram_data;
    int histogram_map[60];          //Histogram存储中介(数据点图)
    void fresh_Histogram();

    //频谱图
    QwtPlot *plot_Spectra;
    QwtPlotHistogram *d_Spectra;
    QVector<QwtIntervalSample> Spectra_data;
    int Spectra_map[60];          //Spectra存储中介(数据点图)
    void fresh_Spectra();
    void do_Spectra_compute();

};

#endif // AEWIDGET_H








