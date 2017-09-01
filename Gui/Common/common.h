#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QComboBox>
#include <qwt_plot.h>
#include <qwt_plot_spectrocurve.h>
#include <qwt_plot_histogram.h>
#include <QLabel>

class Common : public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = 0);

    static void change_index(unsigned char &index, int d_index,     //完成index的循环增减功能
                             int max_index, int min_index);

    static void set_comboBox_style(QComboBox *comboBox);                  //完成QComboBox的样式设置
    static void set_barchart_style(QwtPlot *plot);                  //完成barchart的样式设置
    static void set_PRPD_style(QwtPlot *plot, QwtPlotSpectroCurve *d_PRPD, int max_value);                      //完成PRPD的样式设置
    static void set_histogram_style(QwtPlot *plot, QwtPlotHistogram *d_histogram);                      //完成histogram的样式设置
    static void setTab(QLabel *label);              //设置tabwidget的标签格式
signals:

public slots:
};

#endif // COMMON_H
