#ifndef OPTIONWIDGET_H
#define OPTIONWIDGET_H

#include "TabWidget/optionbasic.h"
#include "TabWidget/optionexpert.h"
#include "TabWidget/optionstorage.h"
#include "TabWidget/optionwifi.h"
#include "../Common/basewidget.h"
#include "IO/Data/data.h"

namespace Ui {
class OptionWidget;
}

class OptionWidget : public BaseWidget
{
    Q_OBJECT
public:
    explicit OptionWidget(G_PARA *g_data, QWidget *parent = nullptr);
    ~OptionWidget();

    void working(CURRENT_KEY_VALUE *val);

signals:
    void fregChanged(float );
    void gpsChanged(QString );
    void show_indicator(bool);          //菊花
    void show_input(QString,QString);   //唤起软键盘
    void send_input_key(quint8);        //专门的虚拟键盘事件
    void send_sync(qint64,qint64);          //发送同步时间（秒，微秒）
    void show_wifi_icon(int);           //显示wifi
    void update_statusBar(QString);

public slots:
    void trans_key(quint8 key_code);
    void input_finished(QString str);

private slots:
    void do_tab_quit();     //处理子页面退出
    void save_options();                    //保存sql

protected:
    void timerEvent(QTimerEvent *e);

private:
    Ui::OptionWidget *ui;

    OptionBasic *option_basic;
    OptionExpert *option_expert;
    OptionStorage *option_storage;
    OptionWifi *option_wifi;

    G_PARA *data;
    SQL_PARA sql_para;

    QLabel *tab0, *tab1, *tab2, *tab3;

    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);

    void fresh();
};

#endif // OPTIONWIDGET_H
