#ifndef CHANNELWIDGET_H
#define CHANNELWIDGET_H

#include <QFrame>
#include <QVector>
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/File/logtools.h"
#include "IO/Com/rdb/rdb.h"

#include "../Common/barchart.h"
#include "../Common/prpsscene.h"
#include "../Common/recwaveform.h"
#include "../Common/common.h"
#include "Algorithm/compute.h"
#include "Algorithm/fft.h"
#include "../Common/basewidget.h"
#include "../Charts/historicchart.h"
#include "../Charts/prpdchart.h"
#include "../Charts/prpschart.h"
#include "../Charts/histogramchart.h"
#include "../Charts/spectrachart.h"
#include "../Charts/tfchart.h"
#include "../Charts/flychart.h"
#include "../Charts/camerachart.h"

#include "../Functions/hchannelfunction.h"
#include "../Functions/lchannelfunction.h"
#include "settingmunu.h"

class QTimer;
class QwtPlotSpectroCurve;
class QwtPlotHistogram;
class QwtPlotCurve;
class QGraphicsView;

#define PEAK_MAX 50
#define EFFECTIVE_MAX 50
#define FREQ50_MAX 5
#define FREQ100_MAX 5

class ChannelWidget : public BaseWidget
{
    Q_OBJECT
public:
    explicit ChannelWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = nullptr);    

public slots:
    void save_channel();            //保存通道数据
    void set_current(int index);    //设置当前通道是否前台
    void change_log_dir();          //改变asset目录

protected slots:
    virtual void showWaveData(VectorList buf, MODE mod);
    virtual void fresh_1000ms();
    virtual void fresh_100ms();
    virtual void trans_key(quint8 key_code);
    void doHideWaveData();          //退出波形显示处理
    void read_short_data();         //读取短录波数据(高频)
    void read_envelope_data(VectorList list);      //读取包络线数据(低频)


signals:
    void startRecWave(MODE, int);
    void send_log_data(double val, int pulse, double degree, int qc, QString r);    //发送日志数据
//    void send_PRPD_data(QVector<QwtPoint3D>);
    void beep(int index, int red_alert);        //蜂鸣器报警(参数：通道，严重程度(0,1,2))

protected:
    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    virtual void fresh_setting();
    virtual void data_reset();

    HChannelFunction *h_fun;
    LChannelFunction *l_fun;
    BaseChannlFunction *fun;

    //常规变量
    G_PARA *data;
    MODE mode;
    int menu_index;     //位于主菜单的位置索引
    LogTools *logtools;

    //定时器
    QTimer *timer_freeze;
    SettingMunu *settingMenu;

    HistoricChart *historic_chart;      //时序图(所有通道)
    PRPDChart *prpd_chart;              //PRPD(所有通道)
    PRPSChart *prps_chart;              //PRPS(所有通道,AA,AE待添加)
    SpectraChart *spectra_chart;        //频谱图(所有通道,TEV待添加)
    HistogramChart *histogram_chart;    //柱状图(TEV强度图,AE特征指数图)    
    TFChart *tf_chart;                  //t-f图(仅HF通道)
    FlyChart *fly_chart;                //飞行图(仅AE通道)
    CameraChart *camera_chart;          //摄像头(仅AA通道)

    //录波
    bool manual;        //手动录波标志
    RecWaveForm *recWaveForm;
};

#endif // CHANNELWIDGET_H
