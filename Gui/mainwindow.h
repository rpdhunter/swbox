#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFrame>
#include <QLabel>
#include <QTimer>
#include <QSplashScreen>
#include <QQuickWidget>
#include <QVector>
#include <QMessageBox>

#include "IO/Data/fifodata.h"
#include "IO/Sync/syncthread.h"
#include "IO/Key/keydetect.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Other/battery.h"
#include "IO/Other/buzzer.h"

#include "Function/tevwidget.h"
#include "Function/hfctwidget.h"
#include "Function/uhfwidget.h"
#include "Function/faultlocation.h"
#include "Function/aawidget.h"
#include "Function/aewidget.h"
#include "Function/Asset/assetwidget.h"

#include "Options/debugset.h"
#include "Options/factoryreset.h"
#include "Options/options.h"
#include "Options/recwavemanage.h"
#include "Options/systeminfo.h"

#include "Common/common.h"
#include "IO/Com/rdb/rdb.h"
#include "IO/Other/CPU/cpustatus.h"
//#include <dwt.h>

//using namespace splab;

namespace Ui {
class MainWindow;
}

class MainWindow : public QFrame
{
    Q_OBJECT

public:
    explicit MainWindow(QSplashScreen *sp, QWidget *parent = 0 );
    ~MainWindow();

public slots:
    void trans_key(quint8 key_code);

signals:
    void send_key(quint8);
    void show_input(QString,QString);
    void send_input_key(quint8);
    void input_str(QString str);

private slots:
    void fresh_menu_icon();
    void fresh_standed_status_message();
    void fresh_status();
    void fresh_batt();
    void system_reboot();
    void system_sleep();
    void screen_dark();
    void set_reboot_time();
    void show_message(QString str);
    void show_busy(bool f);
    void set_wifi_icon(int w);
    void do_beep(int index, int red_alert);
    void set_current_equ(QString new_equ, QString new_path);

    void printSc(); //截屏

private:
    Ui::MainWindow *ui;

    G_PARA *data;
    CURRENT_KEY_VALUE key_val;
    QList<MODE> mode_list;  //维护一个模式表，可以动态地装载组件

    //3个线程
    KeyDetect *keydetect;
    FifoData *fifodata;
    SyncThread *syncThread;

    //12个逻辑通道
    TEVWidget *tev1_widget, *tev2_widget;
    HFCTWidget *hfct1_widget, *hfct2_widget;
//    UHFWidget *uhf1_widget, *uhf2_widget;
    TEVWidget *uhf1_widget, *uhf2_widget;
    FaultLocation *double_widget;
    AAWidget *aa1_widget, *aa2_widget;
    AEWidget *ae1_widget, *ae2_widget;
    AssetWidget *asset_widget;

    //5个设置界面
    Options *options;
    SystemInfo *systeminfo;
    DebugSet *debugset;
    FactoryReset *factoryreset;
    RecWaveManage *recwavemanage;

    //自定义Tab图标
    QLabel *menu_icon0, *menu_icon1, *menu_icon2, *menu_icon3, *menu_icon4, *menu_icon5, *menu_icon6;

    //状态栏
    QTimer *timer_time;                 //系统时间和系统关机倒计时
    QTimer *timer_batt;                 //电池电量监视
    QTimer *timer_reboot;               //系统关机计时器
    QTimer *timer_sleep;                //系统休眠计时器
    QTimer *timer_dark;                 //屏幕暗化计时器
    QTimer *timer_message;              //状态栏信息计时器
    Battery *battery;
    int low_power;                      //自动关机计数
    QVector<int> power_list;            //电量序列

    QQuickWidget *busyIndicator;

    int serial_fd;          //保存一个全局的串口fd值
    Buzzer *buzzer;
    QMessageBox *box;       //关机确认框
    int power_num;          //电源键记数

    void menu_init();
    void statusbar_init();
    void function_init(QSplashScreen *sp);
    void channel_init(MODE mode, int index);
    void options_init();
    void qml_init();
    void set_non_current_menu_icon();       //设置非活动菜单图标
    void set_disable_menu_icon();           //设置禁用动菜单图标
    void fresh_grade1(void);                //刷新设置子选项
    void save_channel();       //保存所有通道的PRPD文件
    void set_asset_dir(QString new_path);       //设置asset路径
    void create_report();       //生成测试报告
    void do_key_up_down(int d);
    void do_key_left_right(int d);

};

#endif // MAINWINDOW_H
















