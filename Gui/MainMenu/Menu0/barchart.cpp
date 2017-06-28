#include "barchart.h"
#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_barchart.h>
#include <qwt_column_symbol.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>

BarChart::BarChart(QwtPlot *parent, int *p, int *high, int *low)
{
    db = p;
//    sql_para = high;
    this->high = high;
    this->low = low;

    barchart_data = new BARCHART_DATA[CHART_NUM];
    attach(parent);
}

void BarChart::addDistro(const QColor &color)
{
    d_colors += color;
    //itemChanged();                                                              //Update the legend of the parent plot.
}

void BarChart::fresh(void)
{
    int i;

    d_colors.clear();
    samples.clear();

    barchart_data[0].color = QColor(Qt::transparent);
    barchart_data[0].height = 0;

    //data left move
    //果然，使用数据左移的方式存储历史数据，实际上就是没有存储
    for (i = 1; i < CHART_NUM - 1; i++) {
        barchart_data[i].height = barchart_data[i + 1].height;
    }

    barchart_data[CHART_NUM - 1].height = *db;
    *db = 0;    //清零（aa超声难以自动清零，把清零语句放在这里）

    for (i = 1; i < CHART_NUM; i++) {
        if (barchart_data[i].height < *low) {
            barchart_data[i].color = QColor(Qt::green);
        } else if (barchart_data[i].height >= *high) {
            barchart_data[i].color = QColor(Qt::red);
        } else {
            barchart_data[i].color = QColor(Qt::yellow);
        }
    }

    //数据填装
    for (i = 0; i < CHART_NUM; i++) {
        samples += barchart_data[i].height;                                     //fit data
        addDistro(barchart_data[i].color);                                      //update color
    }
    setSpacing(4);
    setSamples(samples);                                                        //update
}

QwtColumnSymbol *BarChart::specialSymbol(int sampleIndex, const QPointF& ) const
{
    QwtColumnSymbol *symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
    symbol->setLineWidth(1);
    symbol->setFrameStyle(QwtColumnSymbol::Plain);                              //Raised

    QColor c(Qt::white);
    if (sampleIndex >= 0 && sampleIndex < d_colors.size()) {
        c = d_colors[sampleIndex];
    }
    symbol->setPalette(c);
    return symbol;
}
