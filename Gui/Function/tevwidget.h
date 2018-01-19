#ifndef AMPLITUDE1_H
#define AMPLITUDE1_H

#include <QFrame>
#include <QVector>
#include <QGraphicsView>

#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Other/logtools.h"
#include "IO/Com/rdb/rdb.h"

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


signals:
    void fresh_parent();
    void send_key(quint8);
    void startRecWave(MODE, int);
    void tev_log_data(double val, int pulse, double degree);    //发送日志数据
    void tev_PRPD_data(QVector<QwtPoint3D>);
    void beep(int index, int red_alert);        //蜂鸣器报警(参数：通道，严重程度(0,1,2))

private slots:
    void reload(int index);   //重新装载设置
    void trans_key(quint8 key_code);
    void showWaveData(VectorList buf, MODE mod);

    void fresh_plot(void);
//    void fresh_PRPD();
    void fresh_Histogram();

    void fresh_1000ms();
    void fresh_200ms();
    void fresh_1ms();

    void add_token();


private:
    //常规变量
    Ui::TEVWidget *ui;
    CURRENT_KEY_VALUE *key_val;
    G_PARA *data;
    G_RECV_PARA_SHORT *short_data;
    SQL_PARA sql_para;
    TEV_SQL *tev_sql;
    MODE mode;
    int menu_index;     //位于主菜单的位置索引
    LogTools *logtools;
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void chart_ini();
    void maxReset();        //最大值清零


    void fresh_setting();

    //定时器
    QTimer *timer_1ms, *timer_200ms, *timer_1000ms, *timer_freeze;

    //基本数据
    int db;
    int max_db;
    int db_last1, db_last2;
    quint32 pulse_cnt_last; //上一秒秒冲数
    quint32 group_num;   //用于判别PRPD图数据有效性的组号(0-3变化)
    void calc_tev_value (double * tev_val, double * tev_db, int * sug_central_offset, int * sug_offset);
    QVector<QPoint> pulse_200ms;      //200ms的脉冲数据,分析的基准
    int pulse_number;
    uint token;

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

    //PRPS
    QGraphicsView *plot_PRPS;       //PRPS图
    PRPSScene *scene;

    //柱状图
    QwtPlot *plot_Histogram;
    QwtPlotHistogram *d_histogram;   //Histogram图
    QVector<QwtIntervalSample> histogram_data;

    //录波
    bool manual;        //手动录波标志
    RecWaveForm *recWaveForm;




//    QwtPlotCurve *curve_grid;
//    G_RECV_PARA_PRPD *data_prpd;        //PRPD数据
//    QVector<QPoint> points_origin;      //一次读取到的原始脉冲（用于故障定位）

};

#endif // AMPLITUDE1_H
