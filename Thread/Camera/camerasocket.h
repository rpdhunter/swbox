#ifndef CAMERASOCKET_H
#define CAMERASOCKET_H

#include <QThread>
#include <QTcpSocket>
#include <QTimer>

#define  CAMERA_IP    "192.168.150.49"

#define  RET_OK         100       //返回ok

#define LOGIN_REQ       1000    //登录请求和响应
#define LOGIN_RET       1001

#define LOGIN_OUT_REQ   1001    //登出
#define LOGIN_OUT_RET   1002

#define KEEPALIVE_REQ   1006    //保活
#define KEEPALIVE_RET   1007

#define REALMONITOR_REQ 1410    //实时监视
#define REALMONITOR_RET 1411

#define MONITORDATA     1412   //实时监视数据

#define MONITORCLAIM_REQ  1413   //监视认领
#define MONITORCLAIM_RET  1414

class CameraSocket : public QObject
{
    Q_OBJECT
public:
    explicit CameraSocket(QObject *parent = nullptr);


signals:
    void sendOneFrame(QByteArray packet);

public slots:
    void connect_camera();

private slots:
    void socket_init();         //socket初始化
    void socket_connect();    //断线重连

    void deal_order();
    void deal_data();

private:
    typedef struct _control_flow {
        char Head_Flag;
        char Version;
        char Reserve1;
        char Reserve2;
        int Session_Id;
        int Sequence_Number;
        char Total_Packet;
        char Cur_Packet;
        unsigned short  Message_Id;
        int Data_Length;
    } control_flow;

    typedef struct _media_flow {
        char Head_Flag;
        char Version;
        char Reserve1;
        char Reserve2;
        int Session_Id;
        int Sequence_Number;
        char Channel;
        char End_Flag;
        unsigned short Message_Id;
        int Data_Length;
    } media_flow;

    QTcpSocket *order_socket, *data_socket;
    QThread *thread;
    QTimer *timer;

    void print_info(char *buf, int len, QString name);      //打印信息
    control_flow  *control;
    media_flow  *media;

    QByteArray packet;      //socket有效数据包(最大为8192)
    QByteArray frame;       //有效数据帧(由一包或多包组成,大小不定)

    bool find_media_flow(QByteArray &list);  //寻找数据头

    QByteArray Login_Req(QString user, QString password);
    QByteArray KeepAlive(int id);
    QByteArray Monitor_Claim(int id, QString act, unsigned short mes_id);
    QString phrase(QString s1, QString s2);        //s1,s2拼接成一个短语
    QString phrase(QString s1, int s2);
    QString statement(QStringList list);            //将众多短语拼接成一个语句
    QByteArray add_control_flow(QByteArray data, int id, unsigned short mes_id);

};

#endif // CAMERASOCKET_H
