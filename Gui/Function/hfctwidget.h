#ifndef HFCTWIDGET_H
#define HFCTWIDGET_H

#include <QFrame>
#include <QMap>
#include <qwt_point_3d.h>
#include <QGraphicsView>

#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Other/logtools.h"

#include "../Common/barchart.h"
#include "../Common/recwaveform.h"
#include "../Common/common.h"
#include "../Common/prpsscene.h"

namespace Ui {
class HFCTWidget;
}

class QwtPlot;
class QTimer;
class BarChart;
class QwtPlotSpectroCurve;


class HFCTWidget : public QFrame
{
    Q_OBJECT

public:
    explicit HFCTWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = 0);
    ~HFCTWidget();

public slots:
    void reload(int index);   //重新装载设置
    void trans_key(quint8 key_code);
    void showWaveData(VectorList buf, MODE mod);

signals:
    void fresh_parent();
    void send_key(quint8);
    void startRecWave(MODE, int);
    void hfct_log_data(double,int,double);
    void hfct_PRPD_data(QVector<QwtPoint3D>);
    void show_indicator(bool);      //显示菊花
    void beep(int index, int red_alert);        //蜂鸣器报警

private slots:
    void fresh_1000ms();
    void fresh_100ms();
    void fresh_1ms();
    void add_token();

private:
    struct PC_DATA {
        double pc_value;    //脉冲放电强度
        int phase;          //相位
        double rise_time;      //上升时间
        double fall_time;      //下降时间
    };

    //常规变量
    Ui::HFCTWidget *ui;
    CURRENT_KEY_VALUE *key_val;
    G_PARA *data;
    G_RECV_PARA_SHORT *short_data;
    SQL_PARA sql_para;
    H_CHANNEL_SQL *hfct_sql;
    MODE mode;
    int menu_index;
    LogTools *logtools;
    uint token;

    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void chart_ini();
    void fresh_setting(void);

    //定时器
    QTimer *timer_1ms, *timer_100ms, *timer_1000ms, *timer_freeze;

    //数据流
    quint32 group_num;                  //有效数据校验
    QVector<PC_DATA> pclist_200ms;      //200ms的脉冲数据,HFCT分析的基准
    QVector<PC_DATA> compute_pc_1ms(QVector<double> list, int x_origin);        //计算1ms数据的HFCT数据
    PC_DATA compute_pc_1node(QVector<double> list, int x_origin);           //计算一个脉冲节点的HFCT数据
    double simpson(QVector<double> list);
    double triangle(double d1, double d2);

    //基本数据+时序图
    BarChart *d_barchart;              //时序图
    QwtPlot *plot_BarChart;
    int low, high;
    int db;         //每秒的最大值，用于给图形传递参数
    int pulse_number;       //脉冲数
    double max_db;  //最大值
    void maxReset();

    //PRPD
    QwtPlot *plot_PRPD;             //PRPD窗口部件(qwt)
    QwtPlotSpectroCurve *d_PRPD;    //PRPD曲线(qwt)
    QVector<QwtPoint3D> points_PRPD;//PRPD曲线数据(qwt)
    QMap<MyKey,int> map_PRPD;       //PRPD数据(Qt,points_PRPD的数据源,便与检索)
    void PRPDReset();

    //t-f图
    QwtPlot *plot_TF;
    QwtPlotSpectroCurve *d_TF;    //TF曲线(qwt)
    QVector<QwtPoint3D> points_TF;//TF曲线数据(qwt)
    QMap<MyKey,int> map_TF;       //TF数据(Qt,points_TF的数据源,便与检索)

    //PRPS
    QGraphicsView *plot_PRPS;       //PRPS图
    PRPSScene *scene;

    //柱状图
    QwtPlot *plot_Histogram;
    QwtPlotHistogram *d_histogram;   //Histogram图
    QVector<QwtIntervalSample> histogram_data;

    //录波
    bool isBusy;            //菊花状态
    bool manual;            //手动录波标志
    MODE mode_continuous;
    RecWaveForm *recWaveForm;






    QVector<int> compute_pulse_number(QVector<double> list);

    QVector<QPoint> points_origin;  //原始脉冲点


    QVector<double> pCList;     //记录一秒内的脉冲pc值列表

    double compute_list_pC(QVector<double> list, int x_origin);
    double compute_one_pC(QVector<double> list);


};

#endif // HFCTWIDGET_H
