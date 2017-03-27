#ifndef HFCBARCHART_H
#define HFCBARCHART_H

#include <QDebug>
#include <QTimer>
#include <qwt_plot.h>
#include <qwt_plot_barchart.h>
#include <qstringlist.h>
#include <qwt_column_symbol.h>

typedef struct {
#define CHART_NUM           13
    int height;
    QColor color;
} HFCBARCHART_DATA;

class HFCBarChart : public QFrame, public QwtPlotBarChart
{
    Q_OBJECT
public:
    HFCBarChart(QwtPlot *parent = NULL);

public slots:
    void fresh(void);

private:
    void addDistro(const QColor &color);
    QList<QColor> d_colors;
    QVector<double> samples;

    HFCBARCHART_DATA *barchart_data;
    QTimer *timer;

protected:
    virtual QwtColumnSymbol *specialSymbol(int sampleIndex, const QPointF& ) const;
};

#endif // HFCBARCHART_H
