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

class MyKey{
public:
    MyKey(int X=0,int Y=0){
        x = X;
        y = Y;
    }
    bool operator < (const MyKey another) const
    {
        if(this->x < another.x ){
            return true;
        }
        else if(this->x > another.x){
            return false;
        }
        else if(this->y < another.y){       //this->x == another.x
            return true;
        }
        else {                              //this->x == another.x
            return false;
        }
    }

    bool operator > (const MyKey another) const
    {
        if(this->x > another.x ){
            return true;
        }
        else if(this->x < another.x){
            return false;
        }
        else if(this->y > another.y){       //this->x == another.x
            return true;
        }
        else {                              //this->x == another.x
            return false;
        }
    }

    bool operator == (const MyKey another) const
    {
        if(this->x == another.x && this->y == another.y){
            return true;
        }
        else {
            return false;
        }
    }

    int x;
    int y;


};


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
    void send_key(quint8);
    void startRecWave(MODE, int);        //开始录播
    void fresh_parent();
    void hfct_log_data(double,int,double);
    void hfct_PRPD_data(QVector<QwtPoint3D>);
    void show_indicator(bool);      //显示菊花

private slots:
    void fresh_plot();
    void fresh_PRPD();
    void doHfctData();

private:
    void fresh_setting(void);

    void chart_ini();

    void maxReset();
    void PRPDReset();

    void do_key_up_down(int d);
    void do_key_left_right(int d);


    SQL_PARA sql_para;
    HFCT_SQL *hfct_sql;
    bool manual;        //手动录波标志

    QGraphicsView *plot_PRPS;       //PRPS图
    PRPSScene *scene;

    QwtPlot *plot_BarChart, *plot_PRPD, *plot_Histogram;
    BarChart *d_PRPS;              //PRPS图
    QwtPlotSpectroCurve *d_PRPD;   //PRPD图
//    QwtPlotHistogram *d_histogram;   //Histogram图

//    int map[360][121];
    QMap<MyKey,int> map;


    QVector<QwtPoint3D> points;

    QVector<int> compute_pulse_number(QVector<double> list);
    int pulse_number;
    QVector<QPoint> points_origin;  //原始脉冲点


//    QVector<QwtIntervalSample> histogram_data;

    int db;         //每秒的最大值，用于给图形传递参数
    double max_db;  //最大值
    int low, high;
//    double temp_db; //显示值缓冲区，用于减缓刷新


    CURRENT_KEY_VALUE *key_val;
    QTimer *timer, *timer1 , *timer2, *timer3, *timer_freeze;
    G_PARA *data;
    int menu_index;
    quint32 group;
    QVector<double> pCList;     //记录一秒内的脉冲pc值列表

    double compute_pC(QVector<double> list, int x_origin);
    double compute_one_pC(QVector<double> list);
    double simpson(QVector<double> list);
    double triangle(double d1, double d2);

    RecWaveForm *recWaveForm;
    LogTools *logtools;

    MODE mode, mode_continuous;
    G_RECV_PARA_HFCT *hfct_data;

    bool isBusy;            //菊花状态

    Ui::HFCTWidget *ui;
};

#endif // HFCTWIDGET_H
