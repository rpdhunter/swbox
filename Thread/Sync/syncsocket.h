#ifndef SYNCSOCKET_H
#define SYNCSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include "synccompute.h"
#include <QTimer>

class QTimerEvent;

class SyncSocket : public QObject
{
    Q_OBJECT
public:
    explicit SyncSocket(QObject *parent = nullptr);
    ~SyncSocket();


    bool is_connect();                  //检测同步器是否连接

signals:
    void update_status(QString);        //主动更新状态    
//    void get_sync(float);               //收到同步报文
    void send_sync(qint64,qint64);          //发送同步时间（秒，微秒）
    void send_freq(float);                  //发送频率
    void send_gps_info(GPSInfo *);          //发送GPS报文

public slots:
    void init();

    void deal_read();       //读取从sync_socket接收到的数据包
    void deal_write(char *buf, int len);      //通过sync_socket发送数据包
    void deal_reply();
    void deal_sync();                   //发送sync回复
    void set_sync_source(int source);   //设置同步源

    void re_connect();      //重连

private:
//    int recv_times_count;     //接收次数计数
    QTcpSocket *sync_socket;
    char *recv_buf;
    SyncCompute *sync_compute;

    uint connect_num;       //连接数据(一次成功的连接,connect_num清零,失败3次,则断开socket)

    QTimer *timer, *timer_reconnect;

    QString ip_addr;        //同步器IP地址

};

#endif // SYNCSOCKET_H
