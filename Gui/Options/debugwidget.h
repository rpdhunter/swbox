#ifndef DEBUGWIDGET_H
#define DEBUGWIDGET_H

#include "TabWidget/debugchannelhigh.h"
#include "TabWidget/debugchannellow.h"
#include "TabWidget/debugchannelsetting.h"
#include "TabWidget/debughardware.h"
#include "TabWidget/debugexpert.h"
#include "../Common/basewidget.h"
#include "IO/Data/data.h"
#include <QLabel>
#include <QLineEdit>

namespace Ui {
class DebugWidget;
}

class DebugWidget : public BaseWidget
{
    Q_OBJECT
public:
    explicit DebugWidget(G_PARA *g_data, QWidget *parent = 0);
    ~DebugWidget();

    void working(CURRENT_KEY_VALUE *val); 

signals:
    void update_statusBar(QString);

public slots:
    void trans_key(quint8 key_code);

private slots:
    void do_tab_quit();     //处理子页面退出
    void save_options();                    //保存sql


private:
    Ui::DebugWidget *ui;

    DebugChannelHigh *debug_high;
    DebugChannelLow *debug_low;
    DebugChannelSetting *debug_setting;
    DebugHardware *debug_hardware;
    DebugExpert *debug_expert;

    G_PARA *data;
    SQL_PARA sql_para;

    QLabel *tab0, *tab1, *tab2, *tab3, *tab4;

    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);

    void fresh();

    QWidget *widget_password;        //密码窗口
    QLabel *lab1, *lab2;
    QLineEdit *passwordEdit;
    bool pass;  //查看是否有权进入调试模式
    QString password;   //密码
    QString password_set;   //密码设定值
    void iniPasswordUi();
    void resetPassword();
};

#endif // DEBUGWIDGET_H
