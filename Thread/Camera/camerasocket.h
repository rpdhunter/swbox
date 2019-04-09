#ifndef CAMERASOCKET_H
#define CAMERASOCKET_H

#include <QThread>
#include <QTcpSocket>
#include <QTimer>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define  CAMERA_IP    "192.168.150.49"

#define     RET_OK      100       //返回ok


#define     LOGIN_REQ                   1000    //登录请求和响应
#define     LOGIN_RET                   1001

#define     LOGIN_OUT_REQ                   1001    //登出
#define     LOGIN_OUT_RET                   1002

#define     KEEPALIVE_REQ           1006    //保活
#define     KEEPALIVE_RET            1007

#define     REALMONITOR_REQ     1410    //实时监视
#define     REALMONITOR_RET      1411

#define     MONITORDATA              1412   //实时监视数据

#define     MONITORCLAIM_REQ   1413   //监视认领
#define     MONITORCLAIM_RET    1414





class CameraSocket : public QThread
{
    Q_OBJECT
public:
    explicit CameraSocket(QObject *parent = nullptr);
    void connect_camera();

signals:

public slots:
    void deal_order();
    void deal_data();

    void keep_alive();

protected:
    void run();

private:
    typedef struct  _Camera_DVR{
        int  order_fd;
        int  data_fd;
        struct sockaddr_in  supper_addr;
        int  Order_Socket;                  		//指令socket
        int  Data_Socket;                           //数据socket
        int  sign;
        int  Session_Id;                            //会话ID
        unsigned short  Order_Message_Id;           //消息码
        unsigned short  Data_Message_Id;            //消息码
        int Order_Data_Len;                         //数据长度
        int Data_Data_Len;                          //数据长度
    }Camera_DVR;

    typedef struct _control_flow {
        char Head_Flag;
        char Version;
        char Reserve1;
        char Reserve2;
        int    Session_Id;
        int    Sequence_Number;
        char Total_Packet;
        char Cur_Packet;
        unsigned short  Message_Id;
        int    Data_Length;
    } control_flow;

    typedef struct _media_flow {
        char Head_Flag;
        char Version;
        char Reserve1;
        char Reserve2;
        int    Session_Id;
        int    Sequence_Number;
        char Channel;
        char End_Flag;
        unsigned short  Message_Id;
        int    Data_Length;
    } media_flow;
    QTcpSocket *order_socket, *data_socket;
    QTimer *timer_alive;
    char *recv_buf;
    QByteArray SessionID;

    void thread_init();           //初始化

    void  Login_Req(char *user, char *password, char *data, int &len);

    void  link_one(char *src);
    void  link_two(char *src1, char *src2);
    void  link_two_1(char *src1, char *src2);
    void  link_three(char *src1,char *src2);
    void  link_four(char *src);
    void  link_four_1(char *src);
    void  get_control_flow(char *buf, int id, int seq_num, char tol_pck, char cur_pck, unsigned short mes_id, int data_len);
    int   socket_write (int fd, char * buf, int len);
    void  KeepAlive(int fd, int id);
    void  Monitor_Claim(int fd, int id, char *act, unsigned short mes_id);

};

#endif // CAMERASOCKET_H
