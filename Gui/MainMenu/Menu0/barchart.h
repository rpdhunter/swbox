#ifndef BARCHART_H
#define BARCHART_H

#include "IO/SqlCfg/sqlcfg.h"
#include <QDebug>
#include <qwt_plot.h>
#include <qwt_plot_barchart.h>
#include <qstringlist.h>
#include <qwt_column_symbol.h>

typedef struct {
    //这个宏，定义了棒图显示的数值数量
#define CHART_NUM           13
    int height;
    QColor color;
} BARCHART_DATA;

class BarChart : public QFrame, public QwtPlotBarChart
{
    Q_OBJECT
public:
    BarChart(QwtPlot *parent = NULL, int *p = NULL, int *high = NULL, int *low = NULL);

public slots:
    void fresh(void);

private:
    int *db;
    SQL_PARA *sql_para;
    void addDistro(const QColor &color);
    QList<QColor> d_colors;
    QVector<double> samples;

    BARCHART_DATA *barchart_data;

    int *high,*low;

protected:
    virtual QwtColumnSymbol *specialSymbol(int sampleIndex, const QPointF& ) const;
};

#endif // BARCHART_H
