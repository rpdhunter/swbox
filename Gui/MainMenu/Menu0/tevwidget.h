#ifndef AMPLITUDE1_H
#define AMPLITUDE1_H

#include <QFrame>
#include "barchart.h"
#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <QVector>
#include "../Menu6/DebugSet/recwaveform.h"

namespace Ui {
class Amplitude1;
}

class QTimer;
class QwtPlotSpectroCurve;
class QwtPlotHistogram;
class QwtPlotCurve;

class TEVWidget : public QFrame
{
    Q_OBJECT

public:
    enum Channel{
        Left,
        Right
    };

    explicit TEVWidget(G_PARA *data, Channel channel, QWidget *parent = 0);
    ~TEVWidget();

    void showWaveData(VectorList buf, MODE mod);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8 key_code);

signals:
    void fresh_parent(void);
    void send_key(quint8);
    void offset_suggest(int,int);                               //中心点和噪声建议值
    void tev_modbus_data(int,int);                              //为modbus提供原始数据
    void origin_pluse_points(QVector<QPoint> p, int group);     //p是读取到的脉冲点，group是组号，目的是为故障定位提供原始数据
    void startRecWave(int, int);

private slots:
    void fresh_plot(void);
    void fresh_PRPD();
    void fresh_Histogram();
    void maxReset();        //最大值清零

private:
    Ui::Amplitude1 *ui;

    G_PARA *data;
    CURRENT_KEY_VALUE *key_val;
    int db;
    int max_db;
    quint32 pulse_cnt_last; //上一秒秒冲数
    SQL_PARA sql_para;
    TEV_SQL *tev_sql;

    QTimer *timer1, *timer2 , *timer3;
    quint32 groupNum;   //用于判别PRPD图数据有效性的组号(0-3变化)
    QwtPlot *plot_PRPS, *plot_PRPD, *plot_Histogram;
    BarChart *d_PRPS;              //PRPS图
    QwtPlotSpectroCurve *d_PRPD;   //PRPD图
    QwtPlotHistogram *d_histogram;   //Histogram图

    int map[360][121];
    QwtPlotCurve *curve_grid;

    QVector<QwtPoint3D> points;

    QVector<QPoint> points_origin;      //一次读取到的原始脉冲（用于故障定位）

    QVector<QwtIntervalSample> histogram_data;

    void fresh_setting();
    void PRPS_inti();
    void PRPD_inti();
    void histogram_init();
    void transData(int x, int y);
    void PRPDReset();
    void rec_wave();
	void calc_tev_value (double * tev_val, double * tev_db, int * sug_central_offset, int * sug_offset);
    void reloadSql();   //重新装载设置

//    int temp;
    Channel channel;
//    bool channel_flag;

    RecWaveForm *recWaveForm;
};

#endif // AMPLITUDE1_H
