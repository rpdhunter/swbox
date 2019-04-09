#ifndef RECWAVEFORM_H
#define RECWAVEFORM_H

#include <QWidget>
#include "IO/Key/key.h"
#include "IO/Data/data.h"
#include <QDateTime>

class QwtPlot;
class QwtPlotCurve;
class QwtPlotMarker;

class RecWaveForm : public QWidget
{
    Q_OBJECT

public:
    explicit RecWaveForm(int menu_index, QWidget *parent = 0);
    ~RecWaveForm();

signals:
    void fresh_parent();
    void show_indicator(bool);      //显示菊花

public slots:
    void working(CURRENT_KEY_VALUE *val,QString str);
    void working(CURRENT_KEY_VALUE *val,VectorList buf, MODE mod);     //重载
    void trans_key(quint8 key_code);
    void start_work(VectorList buf, MODE mode);

private:
    struct RecState
    {
        MODE mod;
        QDateTime dateTime;
        QString PB;
        bool wavelet;
        double gain;
        double threshold;
    };
    CURRENT_KEY_VALUE *key_val;
    int menu_index;
    RecState recState;

    QwtPlot *plot;
    QVector<QPointF> wave1, wave2;
    QwtPlotCurve *curve1, *curve2;
    QwtPlotMarker *d_marker_peak, *d_marker_peak_max, *d_marker_peak_min, *d_marker_threshold1, *d_marker_threshold2;

    void setData(VectorList buf, MODE mod);

    void set_canvas();

    void find_peaks();      //寻找峰值
    void show_max();
    void show_min();

    void setScroll(int value);      //根据数值不同，改变显示内容

    void fresh();
    QString get_peak_freq(int current);     //返回坐标i附近的波峰频率
    qreal get_zero(qreal x1, qreal y1, qreal x2, qreal y2);     //寻找过零点,返回过零点的横坐标

    MODE mode;

    int x;              //当前显示的横坐标
    double min, max;    //最大值和最小值（物理值）
    int min_i, max_i;   //最大值和最小值的坐标
    bool max_show_flag, min_show_flag, peak_show_flag;  //最大值、最小值和峰值的显示标志

    double scale;
};

#endif // RECWAVEFORM_H
