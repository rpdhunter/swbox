#ifndef FAULTLOCATION_H
#define FAULTLOCATION_H

#include <QFrame>
#include "IO/Data/data.h"
#include "IO/Key/key.h"
#include "IO/SqlCfg/sqlcfg.h"

class QTimer;

namespace Ui {
class FaultLocation;
}

class QwtPlot;
class QwtPlotCurve;
class FileTools;

class FaultLocation : public QFrame
{
    Q_OBJECT

public:
    explicit FaultLocation(G_PARA *data, QWidget *parent = 0);
    ~FaultLocation();

    void get_origin_points(QVector<QPoint> p,int group);
    void showLeftData(int db,int p);
    void showRightData(int db,int p);

    void showWaveData(VectorList buf, MODE);

public slots:
    void working(CURRENT_KEY_VALUE *val);
    void trans_key(quint8 key_code);


signals:
    void fresh_parent(void);

private slots:
    void setCompassValue();
    void setCompassValue(int c);
    void setMiniCompassValue();

    void processingAScan();     //处理一次扫描
    void startANewScan();    //一次新的扫描
    void showProgress();    //进度条滚动，用于触发时间可视化

private:
    Ui::FaultLocation *ui;

    G_PARA *data;
    CURRENT_KEY_VALUE *key_val;

    SQL_PARA sql_para;

    QTimer *timer, *timer1, *timer2;

    int from,to;
    int speed;
    int process;

    int groupNum_left,groupNum_right;
    QVector<QPoint> points_left, points_right;

    void compare();
    void fresh_setting();

    QwtPlot *plot;
    QVector<QPointF> wave1,wave2;
    QwtPlotCurve *curve1,*curve2;
    double x, min, max;

    double scale;

    VectorList currentData;         //当前显示的数据（用于保存）
    FileTools *filetools;   //文件工具，实现保存文件
    void saveCurrentData();         //保存当前数据至文件

    void setData(VectorList buf);     //设置一次显示数据
    void setScroll(int value);      //根据数值不同，改变显示内容
    void fresh();

//    QTimer *timer;
};

#endif // FAULTLOCATION_H
