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

    QTimer *timer_100ms , *timer_1000ms;
    RecWaveForm *recWaveForm;
    LogTools *logtools;
    bool isBusy;            //菊花状态
    G_RECV_PARA_AE *ae_pulse_data;

    QVector<int> ae_datalist;
    QVector<int> ae_timelist;

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
    int map[360][121];                  //PRPD存储中介(数据点图)
    QPoint transData(int x, int y);     //将原始脉冲转换成可显示的DB值
    void PRPDReset();

    //柱状图
    QwtPlot *plot_Histogram;
    QwtPlotHistogram *d_histogram;   //Histogram图
    QVector<QwtIntervalSample> histogram_data;
    void fresh_Histogram();
};

#endif // AEWIDGET_H
