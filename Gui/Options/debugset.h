#ifndef DEBUGSET_H
#define DEBUGSET_H

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

#include "IO/Key/key.h"
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"


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

private slots:
    void fresh_rdb_data();

private:
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
    QTimer *timer;
    QLabel *tab0, *tab1, *tab2, *tab3;  //tab标签

    QWidget *widget_password;        //密码窗口
    QLabel *lab1, *lab2;
    QLineEdit *passwordEdit;
    bool pass;  //查看是否有权进入调试模式
    QString password;   //密码
    QString password_set;   //密码设定值

    Ui::DebugUi *ui;

};

#endif // DEBUGSET_H
