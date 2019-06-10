#include "historicchart.h"

#define CHART_NUM           13      //定义了棒图显示的数值数量

HistoricChart::HistoricChart(QObject *parent) : BaseChart(parent)
{
}

void HistoricChart::chart_init(QWidget *parent, MODE mode)
{
    BaseChart::chart_init(parent, mode);

    plot = new QwtPlot(parent);
    plot->move(10,35);
    plot->resize(200, 140);
    plot->setStyleSheet("background:transparent;color:gray;");

    plot->setAxisScale(QwtPlot::xBottom, 0, 13);
    plot->setAxisScale(QwtPlot::yLeft, 0, max_value, max_value/3);

    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    plot->axisWidget(QwtPlot::xBottom)->setMargin(0);
    plot->axisWidget(QwtPlot::yLeft)->setMargin(0);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    plot->plotLayout()->setAlignCanvasToScales(true);


    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
    plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    chart = new PlotBarChart;
    chart->setSpacing(4);
    chart->attach(plot);
    reset_colormap();

    for (int i = 0; i < CHART_NUM; i++) {
        samples << 0;
    }

}

void HistoricChart::add_data(int db)
{
    if(db == -100){         //默认-100为无效数据
        return;
    }
    samples.removeFirst();
    samples.append(db);
    chart->setSamples(samples);
    plot->replot();
}

void HistoricChart::save_data()
{

}

void HistoricChart::reset_colormap()
{
    switch (mode) {
    case TEV1:
        chart->set_color_map(sqlcfg->get_para()->tev1_sql.high, sqlcfg->get_para()->tev1_sql.low );
        break;
    case TEV2:
        chart->set_color_map(sqlcfg->get_para()->tev2_sql.high, sqlcfg->get_para()->tev2_sql.low );
        break;
    case HFCT1:
        chart->set_color_map(sqlcfg->get_para()->hfct1_sql.high, sqlcfg->get_para()->hfct1_sql.low );
        break;
    case HFCT2:
        chart->set_color_map(sqlcfg->get_para()->hfct2_sql.high, sqlcfg->get_para()->hfct2_sql.low );
        break;
    case UHF1:
        chart->set_color_map(sqlcfg->get_para()->uhf1_sql.high, sqlcfg->get_para()->uhf1_sql.low );
        break;
    case UHF2:
        chart->set_color_map(sqlcfg->get_para()->uhf2_sql.high, sqlcfg->get_para()->uhf2_sql.low );
        break;
    case AA1:
        chart->set_color_map(sqlcfg->get_para()->aa1_sql.high, sqlcfg->get_para()->aa1_sql.low );
        break;
    case AA2:
        chart->set_color_map(sqlcfg->get_para()->aa2_sql.high, sqlcfg->get_para()->aa2_sql.low );
        break;
    case AE1:
        chart->set_color_map(sqlcfg->get_para()->ae1_sql.high, sqlcfg->get_para()->ae1_sql.low );
        break;
    case AE2:
        chart->set_color_map(sqlcfg->get_para()->ae2_sql.high, sqlcfg->get_para()->ae2_sql.low );
        break;
    default:
        break;
    }
}

void HistoricChart::reset_colormap(int high, int low)
{
    chart->set_color_map(high, low );
}






