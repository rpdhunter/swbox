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
#include "Gui/Common/basewidget.h"


namespace Ui {
class DebugUi;
}

class DebugSet : public BaseWidget
{
    Q_OBJECT
public:
    DebugSet(G_PARA *g_data = NULL, QWidget *parent = NULL);
    void working(CURRENT_KEY_VALUE *val);

public slots:
    void trans_key(quint8 key_code);

signals:
    void fregChanged(int);      //频率设置变化
//    void update_statusBar(QString);

private slots:
    void fresh_rdb_data();

private:


    void iniUi();
    void iniPasswordUi();
    void fresh();       //刷新界面
    void resetPassword();
    void saveSql();     //保存参数
    void reload();      //从SQL读取数据至UI
    void do_key_ok();
    void do_key_cancel();
    void do_key_left_right(int d);
    void do_key_up_down(int d);

    G_PARA *data;
    SQL_PARA sql_para;
    QTimer *timer_rdb;
//    Battery *battery;

    int chk_status_h1, chk_status_h2;       //表示复选框fir和wavelet的选择状态,00代表两复选框都不选择,10代表fir选择,01代表wavelet选择

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
