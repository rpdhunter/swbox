#ifndef CAMERADATA_H
#define CAMERADATA_H

#include <QThread>
#include <QDebug>
#include <QProcess>
#include <QSerialPort>
#include <QThread>
//#include "IO/Com/camerasocket.h"
#include <semaphore.h>
#include <sys/socket.h>
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

#pragma pack(1)
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
#pragma pack()


class CameraData : public QThread
{
    Q_OBJECT
public:
    CameraData(QObject *parent = NULL);
    ~CameraData();

signals:
    void sendOnePacket(QByteArray packet);

protected:
    void run(void);

private:
    QByteArray packet;       //待发送的数据包的缓冲空间
    QByteArray buf_data0, buf_data1,buf_data2;
    int flag;
//    Camera_DVR   camera;

    void send_data(char *data, int len);
};

#endif // CAMERADATA_H
