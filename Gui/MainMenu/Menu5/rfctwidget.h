#ifndef RFCTWIDGET_H
#define RFCTWIDGET_H

#include <QFrame>
#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"

namespace Ui {
class RFCTWidget;
}

class QwtPlot;
class QTimer;
class BarChart;

class RFCTWidget : public QFrame
{
    Q_OBJECT

public:
    explicit RFCTWidget(QWidget *parent = 0, G_PARA *g_data = NULL);
    ~RFCTWidget();

signals:
    void send_key(quint8);
//    void aa_modbus_data(int);
//    void startRecWave(int, int);        //开始录播
//    void offset_suggest(int);
    void fresh_parent();

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8 key_code);
//    void fresh(bool f); //刷新数据核
//    void fresh_1();
//    void fresh_2();

    void doRfctData();

private slots:
    void fresh_plot();

private:
    void fresh_setting(void);
    void PRPS_inti();
//    void maxReset();
//    void calc_aa_value (double * aa_val, double * aa_db, int * offset);

    SQL_PARA *sql_para;

    QwtPlot *plot_PRPS;
    BarChart *d_PRPS;              //PRPS图

    int db;         //每秒的最大值，用于给图形传递参数
//    double max_db;  //最大值
//    double temp_db; //显示值缓冲区，用于减缓刷新


    CURRENT_KEY_VALUE *key_val;
    QTimer *timer1 , *timer2;
    G_PARA *data;
    quint32 group;
    QVector<double> pCList;     //记录一秒内的脉冲pc值列表

    double compute_pC(QVector<double> list);
    double compute_one_pC(QVector<double> list);
    double simpson(QVector<double> list);
    double triangle(double d1, double d2);

private:
    Ui::RFCTWidget *ui;
};

#endif // RFCTWIDGET_H
