#ifndef OPTIONS_H
#define OPTIONS_H

#include <QLabel>
#include <QDateTime>
#include <QListWidget>
#include "IO/SqlCfg/sqlcfg.h"
#include "IO/Data/data.h"
//#include "IO/Other/wifi.h"
//#include "../Common/wifitools.h"
#include "IO/Com/socket.h"
#include "Gui/Common/basewidget.h"
#include "Gui/Common/wifipassword.h"


namespace Ui {
class OptionUi;
class ApInfomation;
}

class Options : public BaseWidget
{
    Q_OBJECT
public:
    Options(QWidget *parent = 0, G_PARA *g_data = NULL);
    ~Options();

    void working(CURRENT_KEY_VALUE *val);

public slots:    
    void trans_key(quint8);
    void input_finished(QString str);

signals:
    void fregChanged(int );
    void closeTimeChanged(int);
    void update_statusBar(QString);

    void show_input(QString,QString);
    void send_input_key(quint8);    //专门的虚拟键盘事件
    void show_wifi_icon(int);

private slots:
    void wifi_aplist(bool f, QStringList list);
    void wifi_hotpot_finished(bool f);
    void wifi_connect_route(bool f, QStringList iplist);

private:
    Ui::OptionUi *ui;
    Ui::ApInfomation *ui_ap;
    QFrame *ap_info_widget;
    int ap_info_widget_button;      //0代表确认,1代表忘记密码

    SQL_PARA sql_para;
    G_PARA *data;
    QDateTime _datetime;
    WifiPassword *wifiPassword;
    QLabel *tab0, *tab1, *tab2, *tab3;

//    WifiTools *tools;
//    int serial_fd;      //保存一个全局的串口fd值
    QListWidget *contextMenu;

    bool inputStatus;       //记录当前是否在虚拟键盘输入状态
    int contextMenu_num;

    Socket *socket;
    QStringList aplist;
    int current_ap_index;       //当前连接的AP序号

    void optionIni();
    void wifi_connect();
    void saveOptions();
    void saveDatetime();
    void refresh();

    void ap_info_widget_init(QString str);

    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);    

};

#endif // OPTIONS_H
