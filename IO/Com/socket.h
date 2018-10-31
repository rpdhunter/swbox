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
    void wifi_aplist();                                 //显示aplist
    void wifi_connect_route(QString name, QString key); //连接一个路由
    void wifi_create_ap(QString name, QString key, QString gateway = "192.168.5.1", QString mask = "255.255.255.0");

    //camera
    void open_camera();         //打开摄像头
    void close_camera();        //关闭摄像头

signals:
    void s_wifi_aplist(bool,QStringList list);          //读取APlist完毕,并发送
    void s_wifi_connect_route(bool, QStringList iplist);    //如果连接成功,iplist返回ip地址,网关,子网掩码,不成功,iplist为空或者是password_is_error
    void s_wifi_create_ap(bool);

    void s_camera_packet(QByteArray, int);           //发送接收到的摄像头数据

public slots:
    void read_data();
    void read_data1();
    void read_done(int f);

private:
    QUdpSocket *mSocket, *mSocket_data;
    QStringList aplist;

    void send_data(QString str, int port);
    QByteArray buf_data;        //用于缓冲组装UDP报文,成组之后发送
//    char *buf0, *buf1, *buf2;
    QByteArray buf_data0, buf_data1,buf_data2;
    int flag;
//    bool flag0, flag1, flag2;
};

#endif // SOCKET_H
