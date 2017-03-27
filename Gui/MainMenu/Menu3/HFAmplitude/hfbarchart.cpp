#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_barchart.h>
#include <qwt_column_symbol.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include "hfbarchart.h"

HFBarChart::HFBarChart(QwtPlot *parent)
{
    barchart_data = new HFBARCHART_DATA[CHART_NUM];
    attach(parent);

    timer = new QTimer(this);
    timer->setInterval(500);
    timer->start();
    connect(timer, &QTimer::timeout, this, &HFBarChart::fresh);
}

void HFBarChart::addDistro(const QColor &color)
{
   d_colors += color;
   //itemChanged();                                                               //Update the legend of the parent plot.
}

void HFBarChart::fresh(void)
{
    d_colors.clear();
    samples.clear();

    barchart_data[0].color = QColor(Qt::transparent);
    barchart_data[0].height = 0;
    barchart_data[1].height = 14;
    barchart_data[2].height = 54;
    barchart_data[3].height = 59;
    barchart_data[4].height = 17;
    barchart_data[5].height = 11;
    barchart_data[6].height = 33;
    barchart_data[7].height = 25;
    barchart_data[8].height = 45;
    barchart_data[9].height = 58;
    barchart_data[10].height = 19;
    barchart_data[11].height = 31;
    barchart_data[12].height = 15;

    for (int i = 1; i < CHART_NUM; i++) {
        if (barchart_data[i].height < 15) {
            barchart_data[i].color = QColor(Qt::yellow);
        } else if (barchart_data[i].height > 45) {
            barchart_data[i].color = QColor(Qt::red);
        } else {
            barchart_data[i].color = QColor(Qt::green);
        }
    }

    for (int i = 0; i < CHART_NUM; i++) {
        samples += barchart_data[i].height;                                     //fit data
        addDistro(barchart_data[i].color);                                      //update color
    }
    setSpacing(4);
    setSamples(samples);                                                        //update
}

QwtColumnSymbol *HFBarChart::specialSymbol(int sampleIndex, const QPointF& ) const
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
