#ifndef PRPSCHART_H
#define PRPSCHART_H

#include "basechart.h"
#include "../Common/prpsscene.h"

class PRPSChart : public BaseChart
{
    Q_OBJECT
public:
    explicit PRPSChart(QObject *parent = nullptr);

    void chart_init(QWidget *parent, MODE mode);               //初始化
    void add_data(QVector<QPoint> point_list);      //增加数据点 QVector<QPoint(相位，幅值)>
//    void reset_data();              //数据重置
    void save_data();               //保存数据到文件

    void reset_colormap();          //重置颜色报警
    void reset_colormap(int high, int low);          //重置颜色报警

    void hide();
    void show();

signals:

public slots:
    void set_data();

protected:
    void timerEvent(QTimerEvent *);

private:
    QGraphicsView *chart;       //PRPS图
    PRPSScene *scene;
    QTimer *timer;
    bool hasData;
};

#endif // PRPSCHART_H
