#ifndef OPTIONWIFI_H
#define OPTIONWIFI_H

#include "tabwidget.h"
#include <QListWidget>
#include "Thread/Wifi/wifimanagement.h"
#include "Thread/Wifi/wifipassword.h"

namespace Ui {
class OptionWifi;
class ApInfomation;
}

class OptionWifi : public TabWidget
{
    Q_OBJECT

public:
    explicit OptionWifi(SQL_PARA *sql, QWidget *parent = 0);
    ~OptionWifi();

    void work(int);
    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);

    bool getInputStatus() const;

public slots:
    void ap_fresh_list_complete();
    void input_finished(QString str);
    void ap_connect_complete(bool f);
    void ap_create_complete(bool f);

signals:
    void quit();        //完全退出的信号
    void save();        //保存设置
    void show_indicator(bool);  //菊花

    void show_input(QString,QString);
    void show_wifi_icon(int);

private:
    Ui::OptionWifi *ui;
    Ui::ApInfomation *ui_ap;
    QFrame *ap_info_widget;
    int ap_info_widget_button;      //0代表确认,1代表忘记密码

    QListWidget *contextMenu;

    bool inputStatus;       //记录当前是否在虚拟键盘输入状态
    int contextMenu_num;
    int menu_row;           //右键菜单选择
    bool isBuzzy;           //菊花繁忙状态，不接受键盘事件
    int hotpot_row;         //热点行

    WifiManagement *wifi_managment;
    WifiPassword *wifi_password;

    void wifi_connect();

    void wifi_init();
    void fresh();

};

#endif // OPTIONWIFI_H
