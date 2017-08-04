#ifndef RECWAVEFORM_H
#define RECWAVEFORM_H

#include <QWidget>
#include "IO/Key/key.h"
#include "IO/Data/data.h"


class QwtPlot;
class QwtPlotCurve;

class RecWaveForm : public QWidget
{
    Q_OBJECT

public:
    explicit RecWaveForm(QWidget *parent = 0);
    ~RecWaveForm();

signals:
    void fresh_parent();

public slots:
    void working(CURRENT_KEY_VALUE *val,QString str);
    void working(CURRENT_KEY_VALUE *val,VectorList buf, MODE mod);     //重载
    void trans_key(quint8 key_code);

private:
    CURRENT_KEY_VALUE *key_val;

    QwtPlot *plot;
    QVector<QPointF> wave1, wave2;
    QwtPlotCurve *curve1, *curve2;

    void setData(QString str);
    void setData(VectorList buf, MODE mod);

    void setScroll(int value);      //根据数值不同，改变显示内容

    void fresh();

    MODE mode;

    double x, min, max;

    double scale;

//    G_PARA *data;
//    SQL_PARA *sql_para;
};

#endif // RECWAVEFORM_H
