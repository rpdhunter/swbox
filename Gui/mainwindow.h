#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFrame>
#include <QLabel>
#include <QTimer>
#include <QSplashScreen>
#include <QQuickWidget>

#include "IO/Data/fifodata.h"
#include "IO/Modbus/modbus.h"
#include "IO/Key/keydetect.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Other/battery.h"

#include "Function/tevwidget.h"
#include "Function/hfctwidget.h"
#include "Function/faultlocation.h"
#include "Function/aawidget.h"
#include "Function/aewidget.h"

#include "Options/debugset.h"
#include "Options/factoryreset.h"
#include "Options/options.h"
#include "Options/recwavemanage.h"
#include "Options/systeminfo.h"

#include "Common/common.h"


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
    void show_input();
    void send_input_key(quint8);
    void input_str(QString str);

private slots:
    void fresh_menu_icon();
    void fresh_status();
    void fresh_batt();
    void system_reboot();
    void set_reboot_time();
    void show_message(QString str);
    void show_busy(bool f);

#ifdef PRINTSCREEN
    void printSc(); //截屏
#endif

private:
    Ui::MainWindow *ui;

    G_PARA *data;
    int grade0_val;
    CURRENT_KEY_VALUE key_val;
    QList<MODE> mode_list;  //维护一个模式表，可以动态地装载组件

    //3个线程
    KeyDetect *keydetect;
    FifoData *fifodata;
    Modbus *modbus;

    //7个逻辑通道
    TEVWidget *tev1_widget, *tev2_widget;
    HFCTWidget *hfct1_widget, *hfct2_widget;
    FaultLocation *double_widget;
    AAWidget *aa_widget;
    AEWidget *ae_widget;        //to be

    //5个设置界面
    Options *options;
    SystemInfo *systeminfo;
    DebugSet *debugset;
    FactoryReset *factoryreset;
    RecWaveManage *recwavemanage;

    //自定义Tab图标
    QLabel *menu_icon0, *menu_icon1, *menu_icon2, *menu_icon3, *menu_icon4, *menu_icon5;

    //状态栏
    QTimer *timer_time;                 //系统时间和系统关机倒计时
    QTimer *timer_batt;                 //电池电量监视
    QTimer *timer_reboot;               //系统关机计时器
    Battery *battery;
    int low_power;                      //自动关机计数

    QQuickWidget *busyIndicator;

    void menu_init();
    void statusbar_init();
    void function_init(QSplashScreen *sp);
    void channel_init(MODE mode, int index);
    void options_init();
    void qml_init();
    void set_non_current_menu_icon();       //设置非活动菜单图标
    void set_disable_menu_icon();           //设置禁用动菜单图标
    void fresh_grade1(void);                //刷新设置子选项

};

#endif // MAINWINDOW_H
















