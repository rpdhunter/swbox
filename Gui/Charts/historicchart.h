#ifndef HISTORICCHART_H
#define HISTORICCHART_H

#include "basechart.h"
#include "Gui/Common/common.h"
#include <qwt_plot_barchart.h>

class PlotBarChart : public QwtPlotBarChart
{
public:
    PlotBarChart(){
        high = 40;
        low = 20;
    }

    void set_color_map(int h, int l){
        high = h;
        low = l;
    }

protected:
    virtual QwtColumnSymbol *specialSymbol(int , const QPointF &sample ) const{
        QwtColumnSymbol *symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
        symbol->setLineWidth(1);
        symbol->setFrameStyle(QwtColumnSymbol::Plain);                              //Raised

        QColor c;
        if(sample.y() < low){
            c = QColor(Qt::green);
        }
        else if(sample.y() < high){
            c = QColor(Qt::yellow);
        }
        else{
            c = QColor(Qt::red);
        }

        symbol->setPalette(c);
        return symbol;
    }

private:
    int high, low;      //颜色变化阈值
};

class HistoricChart : public BaseChart
{
    Q_OBJECT
public:
    explicit HistoricChart(QObject *parent = nullptr);

    void chart_init(QWidget *parent, MODE mode);  //初始化
    void add_data(int db);          //增加数据点
//    void reset_data();              //数据重置
    void save_data();               //保存数据到文件
    void reset_colormap();          //重置颜色报警
    void reset_colormap(int high, int low);          //重置颜色报警

signals:

public slots:

private:
    PlotBarChart *chart;
    QVector<double> samples;
};

#endif // HISTORICCHART_H
