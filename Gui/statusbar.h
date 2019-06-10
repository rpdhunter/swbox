#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QObject>
#include <QTimer>
#include "IO/Other/battery.h"
#include "IO/SqlCfg/sqlcfg.h"

namespace Ui {
class MainWindow;
}

class StatusBar : public QObject
{
    Q_OBJECT
public:
    explicit StatusBar(Ui::MainWindow *_ui, QObject *parent = nullptr);

signals:
    void system_reboot();

public slots:
    void fresh_status();
    void fresh_batt();
    void show_message(QString str);
    void set_wifi_icon(int w);
    void message_reset();               //恢复之前的信息

    void fresh_freq(float f);           //显示频率
    void fresh_miss_freq();         //检测失步
    void fresh_miss_gps();          //检测失去GPS
    void fresh_gps(QString str);

    void turn_dark(bool f);         //屏幕开关

private:
    Ui::MainWindow *ui;

    QTimer *timer_time;                 //系统时间和系统关机倒计时
    QTimer *timer_batt;                 //电池电量监视
    QTimer *timer_message;              //状态栏信息计时器
    QTimer *timer_miss_sync;            //同步失联计时器
    QTimer *timer_miss_gps;            //gps失联计时器

    Battery *battery;
    int low_power;                      //自动关机计数

    float freq;             //实时频率值
    QString gps_info;

    QStringList info_lab_text_list;     //使用堆栈保存显示信息
};

#endif // STATUSBAR_H
