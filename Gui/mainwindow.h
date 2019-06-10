#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFrame>
#include <QTimer>
#include <QSplashScreen>
#include <QQuickWidget>
//#include <QVector>
#include <QMessageBox>

#include "IO/Data/fifodata.h"
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

#include "Options/factoryreset.h"
#include "Options/recwavemanage.h"
#include "Options/systeminfo.h"
#include "Options/optionwidget.h"
#include "Options/debugwidget.h"

#include "Common/common.h"
#include "IO/Com/rdb/rdb.h"
#include "IO/Other/CPU/cpustatus.h"
#include "mainfunction.h"
#include "statusbar.h"

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
    void show_busy(bool f);


private:
    Ui::MainWindow *ui;

    G_PARA *data;
    CURRENT_KEY_VALUE key_val;
    QList<MODE> mode_list;  //维护一个模式表，可以动态地装载组件

    //3个线程
    KeyDetect *keydetect;
    FifoData *fifodata;

    //12个逻辑通道
    TEVWidget *tev1_widget, *tev2_widget;
    HFCTWidget *hfct1_widget, *hfct2_widget;
    UHFWidget *uhf1_widget, *uhf2_widget;
    FaultLocation *double_widget;
    AAWidget *aa1_widget, *aa2_widget;
    AEWidget *ae1_widget, *ae2_widget;
    AssetWidget *asset_widget;

    //5个设置界面
    OptionWidget *option_widget;
    DebugWidget *debug_widget;
    SystemInfo *systeminfo;    
    FactoryReset *factoryreset;
    RecWaveManage *recwavemanage;

    //自定义Tab图标
    QLabel *menu_icon0, *menu_icon1, *menu_icon2, *menu_icon3, *menu_icon4, *menu_icon5, *menu_icon6;

    //状态栏
    Battery *battery;
    int low_power;                      //自动关机计数

    QQuickWidget *busyIndicator;
    QMessageBox *box;       //关机确认框

    MainFunction *main_function;        //从主界面分离出的功能类,完成大部分功能处理
    StatusBar *status_bar;

    void menu_init();
    void function_init(QSplashScreen *sp);
    void channel_init(MODE mode, int index);
    void options_init();
    void qml_init();
    void set_non_current_menu_icon();       //设置非活动菜单图标
    void set_disable_menu_icon();           //设置禁用动菜单图标
    void fresh_grade1(void);                //刷新设置子选项
    void set_asset_dir(QString new_path);       //设置asset路径
    void do_key_up_down(int d);
    void do_key_left_right(int d);

};

#endif // MAINWINDOW_H
















