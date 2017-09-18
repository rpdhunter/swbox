#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QComboBox>
#include <qwt_plot.h>
#include <qwt_plot_spectrocurve.h>
#include <qwt_plot_histogram.h>
#include <QLabel>
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"

#define FREEZE_TIME     5000            //秒界面锁定时间


class Common : public QObject
{
    Q_OBJECT
public:
    explicit Common(QObject *parent = 0);

    static void change_index(unsigned char &index, int d_index,     //完成index的循环增减功能
                             int max_index, int min_index);
    static void change_index(double &index, double d_index,     //完成index的循环增减功能
                             double max_index, double min_index);
    static void change_index(int &index, int d_index,     //完成index的循环增减功能
                             int max_index, int min_index);
    static void change_value(int &value, int value_a, int value_b); //完成value的档位变化
    static void change_value(bool &value, bool value_a, bool value_b); //完成value的档位变化

    static void set_comboBox_style(QComboBox *comboBox);                  //完成QComboBox的样式设置
    static void set_barchart_style(QwtPlot *plot, int v_max);                  //完成barchart的样式设置
    static void set_PRPD_style(QwtPlot *plot, QwtPlotSpectroCurve *d_PRPD, int max_value);                      //完成PRPD的样式设置
    static void set_histogram_style(QwtPlot *plot, QwtPlotHistogram *d_histogram);                      //完成histogram的样式设置
    static void setTab(QLabel *label);              //设置tabwidget的标签格式

    static double physical_value(int code_value, MODE mode);        //根据码值返回物理值
    static int code_value(double physical_value, MODE mode);        //根据物理值返回码值
    static double physical_threshold(MODE mode);                    //返回各模式下的物理阈值
    static QString MODE_toString(MODE val);
signals:

public slots:
};

#endif // COMMON_H
