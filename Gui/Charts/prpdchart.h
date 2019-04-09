#ifndef PRPDCHART_H
#define PRPDCHART_H

#include "basechart.h"

#define PHASE_NUM   360         //相位窗口数
#define AMP_NUM     120         //幅值窗口数

class PRPDChart : public BaseChart
{
    Q_OBJECT
public:
    explicit PRPDChart(QObject *parent = nullptr);

    void chart_init(QWidget *parent, MODE mode);               //初始化
    void add_data(QVector<QPoint> point_list);      //增加数据点 QVector<QPoint(相位，幅值)>
    void reset_data();              //数据重置
    void save_data();               //保存数据到文件
    QVector<QwtPoint3D> samples();  //返回PRPD数据(主要为了供给模式识别)

signals:
//    void send_PRPD_data(QVector<QwtPoint3D>);       //保存PRPD数据

public slots:

private:

    qint32 phase_window;            //量化相位宽度
    float amp_window;              //量化幅值宽度

//    QwtPlot *plot;                 //PRPD图
    QwtPlotSpectroCurve *d_PRPD;        //PRPD曲线
    QVector<QwtPoint3D> prpd_samples;   //PRPD数据
    quint32 map[PHASE_NUM][AMP_NUM];    //PRPD存储中介(数据点图)

    void save_PRPD_zdit();                  //自定义PRPD标准
    void save_PRPD_State_Grid();            //国网公司PRPD标准


};

#endif // PRPDCHART_H
