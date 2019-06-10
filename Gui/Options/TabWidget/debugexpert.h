#ifndef DEBUGEXPERT_H
#define DEBUGEXPERT_H

#include <QTcpSocket>
#include "tabwidget.h"
#include "Thread/Wifi/wifimanagement.h"

namespace Ui {
class DebugExpert;
}

class DebugExpert : public TabWidget
{
    Q_OBJECT

public:
    explicit DebugExpert(SQL_PARA *sql, QWidget *parent = 0);
    ~DebugExpert();

    void work();
    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void fresh();

signals:
//    void sync_matching(QString ssid, QString password);       //同步器匹配
private slots:
    void ap_connect_complete(bool f);       //ap连接成功
    void socket_connected();                //socket建立完成
    void get_reply();                       //收到同步器回复
    void fresh_wifi_time();                 //刷新wifi连接时间

private:
    Ui::DebugExpert *ui;
    WifiManagement *wifi_managment;
    QTcpSocket *tcpSocket;
    QTimer *timer_wifi;     //wifi连接计时器
    uint sec;               //wifi连接读秒

    void ethernet_enable(bool f);

};

#endif // DEBUGEXPERT_H
