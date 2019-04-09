#ifndef SPECTRACHART_H
#define SPECTRACHART_H

#include "basechart.h"
#include "Algorithm/fft.h"

class SpectraChart : public BaseChart
{
    Q_OBJECT
public:
    explicit SpectraChart(QObject *parent = nullptr);

    void chart_init(QWidget *parent, MODE mode);               //初始化
    void add_data(QVector<int> datalist);      //增加数据点 QVector<QPoint(相位，幅值)>
//    void reset_data();              //数据重置
//    void save_data();               //保存数据到文件

signals:

public slots:

private:
    QwtPlotHistogram *d_Spectra;
    QVector<QwtIntervalSample> Spectra_data;
    int Spectra_map[64];          //Spectra存储中介(数据点图)

    FFT *fft;
};

#endif // SPECTRACHART_H
