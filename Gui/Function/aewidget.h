#ifndef AEWIDGET_H
#define AEWIDGET_H

#include "channelwidget.h"

namespace Ui {
class AEWidget;
}

class AEWidget : public ChannelWidget
{
    Q_OBJECT
public:
    explicit AEWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = 0);
    ~AEWidget();

public slots:
    void reload(int index);
    void save_channel();        //保存通道数据

private slots:
    void fresh(bool f); //刷新数据核
    void fresh_1000ms();
    void fresh_100ms();
    void fly_Reset();

private:
    void fresh_setting();
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void do_key_ok();
    void chart_ini();
    void fresh_Histogram();
    void PRPDReset();

    Ui::AEWidget *ui;
    L_CHANNEL_SQL *aeultra_sql;
    double temp_db; //显示值缓冲区，用于减缓刷新
    QTimer *timer_fly;
    QVector<int> ae_fftlist;

    //飞行图
    QwtPlot *plot_fly;                 //飞行图
    QwtPlotSpectroCurve *d_fly;        //飞行曲线
    QVector<QwtPoint3D> fly_samples;   //飞行数据
    int fly_number;                    //飞行周期计数（目前20个周期重置）

    //柱状图
    int histogram_map[60];          //Histogram存储中介(数据点图)

};

#endif // AEWIDGET_H








