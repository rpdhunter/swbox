#ifndef DEBUGSET_H
#define DEBUGSET_H

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QTime>

#include "IO/Key/key.h"
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Other/CPU/cpustatus.h"
#include "IO/Other/battery.h"


namespace Ui {
class DebugUi;
}

class DebugSet : public QFrame
{
    Q_OBJECT
public:
    DebugSet(G_PARA *g_data = NULL, QWidget *parent = NULL);
    void working(CURRENT_KEY_VALUE *val);

public slots:
    void trans_key(quint8 key_code);

signals:
    void fresh_parent();
    void fregChanged(int);      //频率设置变化
    void send_key(quint8);
    void update_statusBar(QString);
    void update_syncBar(bool);  //参数为false代表同步未成功,true为同步成功
    void send_sync(qint64, qint64);     //发送过零点的秒和微秒(linux timeval格式)

private slots:
    void fresh_rdb_data();
    void fresh_hardware_status();
    void fresh_sync();

private:
    struct SYNC_DATA{
        struct timeval t;
        float vcc;
    };

    void iniUi();
    void iniPasswordUi();
    void fresh();       //刷新界面
    void resetPassword();
    void saveSql();
    void reload(); //从SQL读取数据至UI
    void do_key_left_right(int d);
    void do_key_up_down(int d);


    CURRENT_KEY_VALUE *key_val;
    G_PARA *data;
    SQL_PARA sql_para;
    QTimer *timer_rdb, *timer_hardware;
    QTimer *timer_sync;                 //同步信号计时器
    CPUStatus *cpu_status;
    Battery *battery;
    float sync_vcc;                     //同步信号电压
    QVector<SYNC_DATA> sync_data;       //同步数据信号
    QVector<timeval> zero_times;         //保存之前一系列过零点

    QLabel *tab0, *tab1, *tab2, *tab3, *tab4;  //tab标签

    QWidget *widget_password;        //密码窗口
    QLabel *lab1, *lab2;
    QLineEdit *passwordEdit;
    bool pass;  //查看是否有权进入调试模式
    QString password;   //密码
    QString password_set;   //密码设定值

    Ui::DebugUi *ui;

};

#endif // DEBUGSET_H
