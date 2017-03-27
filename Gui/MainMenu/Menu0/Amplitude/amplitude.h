#ifndef AMPLITUDE_H
#define AMPLITUDE_H

#include "barchart.h"
#include "IO/Data/fifodata.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <QTimer>
#include <QFrame>
#include <QDebug>
#include <QLabel>
#include <qwt_plot.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_multi_barchart.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_barchart.h>
#include <qwt_column_symbol.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_histogram.h>
#include <qwt_scale_widget.h>
#include <qwt_series_store.h>
#include <qwt_series_data.h>
#include <qwt_samples.h>

//幅值监测模式
class Amplitude : public QFrame
{
    Q_OBJECT
public:
    Amplitude(QWidget *parent = NULL, G_PARA *g_data = NULL);
    void sysReset();

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8);
    void fresh_plot(void);
    void maxReset();        //最大值清零

signals:
    void fresh_parent(void);
    void offset_suggest(int,int);

private:
    void fresh_setting(void);

//    int offset1,offset2;        //

    G_PARA *data;
    int db;
    int max_db;
    QTimer *timer;
    QwtPlot *plot;
    BarChart *d_chart;
    CURRENT_KEY_VALUE *key_val;




    //脉冲视图
    quint32 pulse_cnt;      //脉冲计数
    quint32 signal_pulse_cnt;
    double degree;          //严重度

    SQL_PARA *sql_para;
    AMP_SQL *amp_sql;

    QLabel *set_lab, *set_child0_lab, *set_child1_lab, *set_child2_lab, *set_child3_lab;
    QLabel *child0_val_lab, *child1_val_lab, *child2_val_lab;
    QLabel *set_txt_lab;
    QLabel *tev_lab, *max_val_lab, *yellow_lab, *green_lab, *red_lab,
        *amp_his_val, *db_lab;
    QLabel *pulse_cnt_lab, *degree_lab;
};

#endif // AMPLITUDE_H
