#ifndef AMPLITUDE1_H
#define AMPLITUDE1_H

#include <QFrame>
#include <QVector>
#include <QGraphicsView>

#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Other/logtools.h"
#include "IO/rdb/rdb.h"

#include "../Common/barchart.h"
#include "../Common/prpsscene.h"
#include "../Common/recwaveform.h"
#include "../Common/common.h"

namespace Ui {
class TEVWidget;
}

class QTimer;
class QwtPlotSpectroCurve;
class QwtPlotHistogram;
class QwtPlotCurve;

class TEVWidget : public QFrame
{
    Q_OBJECT

public:
    explicit TEVWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = 0);
    ~TEVWidget();

public slots:
    void reload(int index);   //重新装载设置
    void trans_key(quint8 key_code);    
    void showWaveData(VectorList buf, MODE mod);

signals:
    void fresh_parent();
    void send_key(quint8);
    void startRecWave(MODE, int);
    void tev_log_data(double val, int pulse, double degree);    //发送日志数据
    void tev_PRPD_data(QVector<QwtPoint3D>);

    void origin_pluse_points(QVector<QPoint> p, int group);     //p是读取到的脉冲点，group是组号，目的是为故障定位提供原始数据


private slots:
    void fresh_plot(void);
    void fresh_PRPD();
    void fresh_Histogram();
    void maxReset();        //最大值清零

private:
    Ui::TEVWidget *ui;

    G_PARA *data;
    CURRENT_KEY_VALUE *key_val;
    int menu_index;     //位于主菜单的位置索引
    MODE mode;
    bool manual;        //手动录波标志
    RecWaveForm *recWaveForm;
    LogTools *logtools;

    int db;
    int max_db;
    int db_last1, db_last2;
    quint32 pulse_cnt_last; //上一秒秒冲数
    SQL_PARA sql_para;
    TEV_SQL *tev_sql;

    QTimer *timer1, *timer2 , *timer_freeze;
    quint32 groupNum;   //用于判别PRPD图数据有效性的组号(0-3变化)
    QwtPlot *plot_Barchart, *plot_PRPD, *plot_Histogram;
    QGraphicsView *plot_PRPS;
    PRPSScene *scene;
    BarChart *d_PRPS;              //PRPS图
    QwtPlotSpectroCurve *d_PRPD;   //PRPD图
    QwtPlotHistogram *d_histogram;   //Histogram图

    int map[360][121];
    QwtPlotCurve *curve_grid;

    QVector<QwtPoint3D> points;

    QVector<QPoint> points_origin;      //一次读取到的原始脉冲（用于故障定位）

    QVector<QwtIntervalSample> histogram_data;
    G_RECV_PARA_PRPD *data_prpd;

    void transData(int &x, int &y);
    void PRPDReset();
	void calc_tev_value (double * tev_val, double * tev_db, int * sug_central_offset, int * sug_offset);
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void chart_ini();

    void fresh_setting();

};

#endif // AMPLITUDE1_H
