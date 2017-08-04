#ifndef RFCTWIDGET_H
#define RFCTWIDGET_H

#include <QFrame>
#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <QMap>
#include "../Menu6/DebugSet/recwaveform.h"
#include "IO/Data/logtools.h"
#include <qwt_point_3d.h>

namespace Ui {
class RFCTWidget;
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


class RFCTWidget : public QFrame
{
    Q_OBJECT

public:
    explicit RFCTWidget(QWidget *parent = 0, G_PARA *g_data = NULL);
    ~RFCTWidget();
    void showWaveData(VectorList buf, MODE mod);

signals:
    void send_key(quint8);
//    void aa_modbus_data(int);
    void startRecWave(MODE, int);        //开始录播
//    void offset_suggest(int);
    void fresh_parent();
    void rfct_log_data(double,int,double);
    void rfct_PRPD_data(QVector<QwtPoint3D>);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8 key_code);
//    void fresh(bool f); //刷新数据核
//    void fresh_1();
//    void fresh_2();

    void doRfctData();

private slots:
    void fresh_plot();
    void fresh_PRPD();
    void rec_wave();
    void rec_wave_continuous_complete();     //连续录波结束


private:
    void fresh_setting(void);
//    void data_init();   //用于延时加载
    void PRPS_inti();
    void PRPD_inti();

    void maxReset();
    void PRPDReset();

    void rec_wave_continuous();     //连续录波

//    void calc_aa_value (double * aa_val, double * aa_db, int * offset);

    SQL_PARA sql_para;

    QwtPlot *plot_PRPS, *plot_PRPD, *plot_Histogram;
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
//    double temp_db; //显示值缓冲区，用于减缓刷新


    CURRENT_KEY_VALUE *key_val;
    QTimer *timer, *timer1 , *timer2, *timer3, *timer_rec, *timer_rec_interval;
    G_PARA *data;
    quint32 group;
    QVector<double> pCList;     //记录一秒内的脉冲pc值列表

    double compute_pC(QVector<double> list, int x_origin);
    double compute_one_pC(QVector<double> list);
    double simpson(QVector<double> list);
    double triangle(double d1, double d2);

    RecWaveForm *recWaveForm;
    LogTools *logtools;

    VectorList rfct_continuous_buf;     //保存收到的连续录波数据

    MODE current_mode;

    Ui::RFCTWidget *ui;
};

#endif // RFCTWIDGET_H
