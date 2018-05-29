#ifndef AAWIDGET_H
#define AAWIDGET_H

#include <QFrame>
#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Other/logtools.h"

#include "../Common/barchart.h"
#include "../Common/recwaveform.h"
#include "../Common/common.h"

namespace Ui {
class AAWidget;
}

class QTimer;
class FFT;

class AAWidget : public QFrame
{
    Q_OBJECT
public:
    explicit AAWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = 0);
    ~AAWidget();

public slots:
    void reload(int index);
    void trans_key(quint8 key_code);
    void showWaveData(VectorList buf, MODE mod);
    void change_log_dir();      //改变asset目录
    void add_ae_data();     //处理256个点的AE数据（1/160秒的数据）
    void save_channel();        //保存通道数据

signals:
    void send_key(quint8);
    void fresh_parent();
    void startRecWave(MODE, int);        //开始录播
    void aa_log_data(double,int,double);
    void show_indicator(bool);      //显示菊花
    void beep(int index, int red_alert);       //蜂鸣器报警

private slots:
    void fresh(bool f); //刷新数据核
    void fresh_1000ms();
    void fresh_100ms();


private:
    void fresh_setting();
    void maxReset();
//    void calc_aa_value (double * aa_val, double * aa_db, int * offset);
    void do_key_up_down(int d);
    void do_key_left_right(int d);

    int db;         //每秒的最大值，用于给图形传递参数
    double max_db;  //最大值
    double temp_db; //显示值缓冲区，用于减缓刷新

    G_RECV_PARA_AE *ae_pulse_data;

    QVector<int> ae_datalist;
    QVector<int> ae_timelist;

    Ui::AAWidget *ui;
    CURRENT_KEY_VALUE *key_val;
    G_PARA *data;
    int menu_index;
    SQL_PARA sql_para;
    L_CHANNEL_SQL *aaultra_sql;
    MODE mode;

    QTimer *timer_100ms , *timer_1000ms;
    BarChart *chart;
    RecWaveForm *recWaveForm;
    LogTools *logtools;
    bool isBusy;            //菊花状态

    //频谱图
    QwtPlot *plot_Spectra;
    QwtPlotHistogram *d_Spectra;
    QVector<QwtIntervalSample> Spectra_data;
    int Spectra_map[60];          //Spectra存储中介(数据点图)
    void do_Spectra_compute();
    FFT *fft;

};

#endif // AAWIDGET_H
