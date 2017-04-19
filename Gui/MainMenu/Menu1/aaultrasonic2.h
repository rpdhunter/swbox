#ifndef AAULTRASONIC2_H
#define AAULTRASONIC2_H

#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <math.h>
#include <QFrame>
#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QProgressBar>

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
#include <qwt_plot_curve.h>

class AAUltrasonic2 : public QFrame
{
    Q_OBJECT
public:
    AAUltrasonic2(QWidget *parent = NULL, G_PARA *g_data = NULL);
    void sysReset();
    void maxReset();
public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8 key_code);
    void fresh(bool f); //刷新数据核
    void fresh_1();
    void fresh_2();
    void fresh_PRPD();  //处理PRPD图

private:
    void fresh_setting(void);
    void fresh_setting_text(void);

    SQL_PARA *sql_para;
    AAULTRA_SQL *aaultra_sql;



    double max_db;  //最大值
    double temp_db; //显示值缓冲区，用于减缓刷新

    //参数设置图标
    QLabel *set_lab, *set_child0_lab, *set_child1_lab, *set_child2_lab, *set_child3_lab,*set_child4_lab;
    //参数设置数值
    QLabel *child0_val_lab, *child1_val_lab, *child2_val_lab, *child3_val_lab;

    QLabel *set_txt_lab;

    QLabel *aaVal_lab,*aaDb_lab, *aaValMax_lab;
    //4个数值滚动条，幅值0-100

    QProgressBar *progbar;
    QLabel *lab_range;   //单位
//    QProgressBar *progbar0, *progbar1, *progbar2, *progbar3;
    //4个数值滚动条所对应的4组显示信息
//    QLabel *lab0[4], *lab1[4], *lab2[4], *lab3[4];

    CURRENT_KEY_VALUE *key_val;

    QTimer *timer1 , *timer2;

    G_PARA *data;

    QwtPlot *plot;

    QwtPlotCurve *curve_green, *curve_yellow, *curve_red, *curve_grid;

    quint32 groupNum;   //用于判别PRPD图数据有效性的组号(0-3变化)

    QVector<double> X_green,Y_green,X_yellow,Y_yellow,X_red,Y_red;

    void transData(int x, int y);
};

#endif // AAULTRASONIC2_H
