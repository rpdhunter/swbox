#ifndef BASECHART_H
#define BASECHART_H

#include <QObject>
#include "Gui/Common/common.h"
#include <qwt_color_map.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include "IO/SqlCfg/sqlcfg.h"

class BaseChart : public QObject
{
    Q_OBJECT
public:
    explicit BaseChart(QObject *parent = nullptr);

    virtual void chart_init(QWidget *, MODE mode);               //初始化
    virtual void hide();
    virtual void show();

signals:

public slots:

protected:
    QwtPlot *plot;              //图形控件
    MODE mode;                      //通道模式
    qint32 max_value;               //最大值
    qint32 min_value;               //最小值
};

#endif // BASECHART_H
