#ifndef MAINFUNCTION_H
#define MAINFUNCTION_H

#include <QObject>
#include <QTimer>
#include "IO/Data/data.h"
#include "IO/Other/buzzer.h"

class MainFunction : public QObject
{
    Q_OBJECT
public:
    explicit MainFunction(G_PARA *_data, QObject *parent = nullptr);

    void do_print_screen();        //截屏操作
    void set_asset_dir(QString new_path);       //设置asset路径

signals:
    void show_message(QString);
    void save_channel();        //保存各通道
    void turn_dark(bool);           //屏幕变暗的信号
    void asset_dir_changed();       //资产路径变化信号

public slots:
    void system_reboot();
    void system_sleep();
    void screen_dark();
    void set_reboot_time();

    void do_beep(int menu_index, int red_alert);
    void reset_power_num();     //重置电源键计数

protected:
    void timerEvent(QTimerEvent *);

private:
    G_PARA *data;

    QTimer *timer_reboot;               //系统关机计时器
    QTimer *timer_sleep;                //系统休眠计时器
    QTimer *timer_dark;                 //屏幕暗化计时器
    QTimer *timer_printSc;              //截屏计时器

    Buzzer *buzzer;

    int power_num;          //电源键记数

    void time_init();       //执行时间检查
    void save();            //执行保存

    void printSc();             //截屏核心函数

};

#endif // MAINFUNCTION_H
