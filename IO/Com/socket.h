#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>
#include <QUdpSocket>
#include "IO/Data/data.h"

class Socket : public QObject
{
    Q_OBJECT
public:
    explicit Socket(QObject *parent = nullptr);

    //wifi
    void wifi_set_baud(int b);                          //配置wifi波特率
    void wifi_set_mode(WIFI_MODE mode);                 //APSTA模式
    void wifi_aplist();                                 //显示aplist
    void wifi_connect_route(QString name, QString key); //连接一个路由
    void wifi_create_ap(QString name, QString key, QString gateway = "192.168.5.1", QString mask = "255.255.255.0");

    //telnet
    void telnet_start(int baud = 115200, int port = 6142);
    void telnet_stop();

    //ftp
    void ftp_start(int baud = 115200, int port = 6141);
    void ftp_stop();

    //104
    void _104_start(int baud = 115200, int port = 6143);
    void _104_stop();

signals:
    void s_wifi_set_baud(bool);
    void s_wifi_set_mode(bool);
    void s_wifi_aplist(bool,QStringList list);          //读取APlist完毕,并发送
    void s_wifi_connect_route(bool, QStringList iplist);    //如果连接成功,iplist返回ip地址,网关,子网掩码,不成功,iplist为空或者是password_is_error
    void s_wifi_create_ap(bool);

public slots:
    void read_data();

private:
    QUdpSocket *mSocket;
    QStringList aplist;

    void send_data(QString str);
};

#endif // SOCKET_H
