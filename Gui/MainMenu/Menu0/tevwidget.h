#ifndef AMPLITUDE1_H
#define AMPLITUDE1_H

#include <QFrame>
#include "barchart.h"
#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <QVector>

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
    explicit TEVWidget(G_PARA *data, QWidget *parent = 0);
    ~TEVWidget();
    void sysReset();

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8 key_code);
    void maxReset();        //最大值清零

signals:
    void fresh_parent(void);
    void offset_suggest(int,int);

private slots:
    void fresh_plot(void);
    void fresh_PRPD();
    void fresh_Histogram();


private:
    Ui::Amplitude1 *ui;

    G_PARA *data;
    CURRENT_KEY_VALUE *key_val;
    int db;
    int max_db;
    SQL_PARA *sql_para;
    AMP_SQL *amp_sql;

    QTimer *timer1, *timer2;
    quint32 groupNum;   //用于判别PRPD图数据有效性的组号(0-3变化)
    QwtPlot *plot_PRPS, *plot_PRPD, *plot_Histogram;
    BarChart *d_PRPS;              //PRPS图
    QwtPlotSpectroCurve *d_PRPD;   //PRPD图
    QwtPlotHistogram *d_histogram;   //Histogram图

    int map[360][121];
    QwtPlotCurve *curve_grid;

    QVector<QwtPoint3D> points;

    QVector<QwtIntervalSample> histogram_data;

    void fresh_setting();
    void PRPS_inti();
    void PRPD_inti();
    void histogram_init();
    void transData(int x, int y);
    void PRPDReset();

    int temp;
};

#endif // AMPLITUDE1_H
