#ifndef AAWIDGET_H
#define AAWIDGET_H

#include <QFrame>
#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "../Menu6/DebugSet/recwaveform.h"

namespace Ui {
class AAWidget;
}

class QTimer;
class BarChart;

class AAWidget : public QFrame
{
    Q_OBJECT

public:
    explicit AAWidget(QWidget *parent = 0, G_PARA *g_data = NULL);
    ~AAWidget();

    void showWaveData(VectorList buf, MODE mod);

signals:
    void send_key(quint8);
    void aa_modbus_data(int);
    void startRecWave(int, int);        //开始录播
    void offset_suggest(int);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8 key_code);
    void fresh(bool f); //刷新数据核
    void fresh_1();
    void fresh_2();

private:
    void fresh_setting(void);
    void maxReset();

    SQL_PARA *sql_para;

    int db;         //每秒的最大值，用于给图形传递参数
    double max_db;  //最大值
    double temp_db; //显示值缓冲区，用于减缓刷新


    CURRENT_KEY_VALUE *key_val;
    QTimer *timer1 , *timer2;
    G_PARA *data;
    BarChart *chart;
    RecWaveForm *recWaveForm;

private:
    Ui::AAWidget *ui;
};

#endif // AAWIDGET_H
