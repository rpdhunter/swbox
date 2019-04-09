#include "camerasocket.h"

CameraSocket::CameraSocket(QObject *parent) : QThread(parent)
{
    timer_alive = new QTimer();
    timer_alive->setInterval(5000);   //10秒1跳
    timer_alive->start();
    connect(timer_alive, SIGNAL(timeout()), this, SLOT(keep_alive()) );

    this->start(QThread::TimeCriticalPriority);
}

void CameraSocket::connect_camera()
{
    if(order_socket->state() == QAbstractSocket::ConnectedState){
        char *data = new char[200];
        int len;
        Login_Req("admin", "tlJwpbo6", data, len);
        qDebug()<<"test len =" << len;

        for(uint i=0;i<len;i++)
        {
            printf("%c", data[i]);
        }
        printf("\n");
        order_socket->write(data, len);
    }
}

void CameraSocket::run()
{
    thread_init();
    while (1) {
        //(重新)建立连接
        order_socket->connectToHost(CAMERA_IP, 34567);
        if (!order_socket->waitForConnected(3 * 1000)){
            qDebug()<<"order_socket failed!";
        }
//        data_socket->connectToHost(CAMERA_IP, 34567);       //不同socket连接同端口服务器
//        if (!order_socket->waitForConnected(3 * 1000)){
//            qDebug()<<"data_socket failed!";
//        }

        //处于连接状态,收发数据
        while (order_socket->state() == QAbstractSocket::ConnectedState ){
//                QByteArray array = order_socket->readAll();
//                if(array.isEmpty()){
//                    msleep(1);
//                }
//                else{
//                    qDebug()<<"get order data:"<<array;
//                }


            order_socket->waitForReadyRead(500);

        }
    }
}

void CameraSocket::thread_init()
{
    order_socket = new QTcpSocket;
    connect(order_socket, SIGNAL(readyRead()), this, SLOT(deal_order()));

    data_socket = new QTcpSocket;
    connect(data_socket, SIGNAL(readyRead()), this, SLOT(deal_data()));

    recv_buf = new char[3000];

}

void CameraSocket::deal_order()
{
    QByteArray array = order_socket->readAll();

    qDebug()<<"get order data: len = " << array.count() << array;

    int index = array.indexOf("SessionID");
    qDebug()<<"SessionID = "<< array.mid(index+14, 10);

    SessionID = array.mid(index+14, 10);

}

void CameraSocket::deal_data()
{

}

void CameraSocket::keep_alive()
{
    int fd = 0;
    int id = 0;
    char buf[200];
    char name[100];
    char sess_id[50];

    char se_id[10];
    sprintf(se_id,"0x000000%02x",id);

    strcpy(name,"Name");
    strcpy(sess_id,"SessionID");

    link_two(name, "KeepAlive");
    link_two(sess_id, se_id);

    link_three(name, sess_id);

    link_four(name);

    get_control_flow(buf, id, 0x00, 0x00,  0x00, KEEPALIVE_REQ, strlen(name));
    strcpy(buf+sizeof(struct _control_flow),name);
#ifdef PRINTABLE
    printf("name buf \n");

    for(uint i=0;i<(sizeof(struct _control_flow) + strlen(name));i++)
    {
        printf("%c", buf[i]);
    }
    printf("\n");
#endif
    socket_write (fd, buf, sizeof(struct _control_flow) + strlen(name));
}



void CameraSocket::Login_Req(char *user, char *password, char *data, int &len)
{
    char buf[200];

    char encry[200];
    char login[50];
    char pass[50];
    char username[50];

    strcpy(encry,"EncryptType");
    strcpy(login,"LoginType");
    strcpy(pass,"PassWord");
    strcpy(username,"UserName");

    link_two(encry, "MD5");
    link_two(login, "DVRIP-Web");
    link_two(pass, password);
    link_two(username, user);

    link_three(encry, login);
    link_three(encry, pass);
    link_three(encry, username);

    link_four(encry);

    get_control_flow(buf, 0x00, 0x00, 0x00,  0x00, LOGIN_REQ, strlen(encry));
    strcpy(buf+sizeof(struct _control_flow),encry);

#ifdef PRINTABLE
    printf("encry buf \n");

    for(uint i=0;i<(sizeof(struct _control_flow) + strlen(encry));i++)
    {
        printf("%c", buf[i]);
    }
    printf("\n");
#endif

//    socket_write (fd, buf, sizeof(struct _control_flow) + strlen(encry));
    len = sizeof(struct _control_flow) + strlen(encry);
    memcpy(data, buf, len);


}

void CameraSocket::link_one(char *src)
{
    char *p;
    char a[200]="\"";
    p = strcat(strcat(a,src),"\"");
    strcpy(src,p);
}

void CameraSocket::link_two(char *src1, char *src2)
{
    char *p;
    char src1_buf[200];
    char src2_buf[200];
    strcpy(src1_buf,src1);
    link_one(src1_buf);
    strcpy(src2_buf,src2);
    link_one(src2_buf);

    p = strcat(strcat(src1_buf," : "),src2_buf);
    strcpy(src1,p);
}

void CameraSocket::link_two_1(char *src1, char *src2)
{
    char *p;
    char src1_buf[200];
    char src2_buf[200];
    strcpy(src1_buf,src1);
    link_one(src1_buf);
    strcpy(src2_buf,src2);

    p = strcat(strcat(src1_buf," : "),src2_buf);
    strcpy(src1,p);
}

void CameraSocket::link_three(char *src1, char *src2)
{
    char *p;
    p = strcat(strcat(src1,", "),src2);
    strcpy(src1,p);
}

void CameraSocket::link_four(char *src)
{
    char p[200];
    strcpy(p,"{ ");
    strcat(p,src);
    strcat(p," }\n");
    strcpy(src,p);
}

void CameraSocket::link_four_1(char *src)
{
    char p[200];
    strcpy(p,"{ ");
    strcat(p,src);
    strcat(p," }");
    strcpy(src,p);
}

void CameraSocket::get_control_flow(char *buf, int id, int seq_num, char tol_pck, char cur_pck, unsigned short mes_id, int data_len)
{
    control_flow  control;

    control.Head_Flag = 0xFF;
    control.Version = 0x00;
    control.Reserve1 = 0x00;
    control.Reserve2 = 0x00;

    control.Session_Id = id;
    control.Sequence_Number = seq_num;
    control.Total_Packet = tol_pck;
    control.Cur_Packet = cur_pck;
    control.Message_Id = mes_id;
    control.Data_Length = data_len;

    memcpy(buf,(char *)&control,sizeof(struct _control_flow));
}

int CameraSocket::socket_write(int fd, char *buf, int len)
{

}

void CameraSocket::KeepAlive(int fd, int id)
{
    char buf[200];
    char name[100];
    char sess_id[50];

    char se_id[10];
    sprintf(se_id,"0x000000%02x",id);

    strcpy(name,"Name");
    strcpy(sess_id,"SessionID");

    link_two(name, "KeepAlive");
    link_two(sess_id, se_id);

    link_three(name, sess_id);

    link_four(name);

    get_control_flow(buf, id, 0x00, 0x00,  0x00, KEEPALIVE_REQ, strlen(name));
    strcpy(buf+sizeof(struct _control_flow),name);
#ifdef PRINTABLE
    printf("name buf \n");

    for(uint i=0;i<(sizeof(struct _control_flow) + strlen(name));i++)
    {
        printf("%c", buf[i]);
    }
    printf("\n");
#endif
    socket_write (fd, buf, sizeof(struct _control_flow) + strlen(name));

}

void CameraSocket::Monitor_Claim(int fd, int id, char *act, unsigned short mes_id)
{
    char buf[250];

    char name[250];
    char opmonitor[200];
    char action[200];

    char para[200];
    char channel[200];
    char combin[30];
    char stream[30];
    char tran[30];

    char session[30];

    char se_id[6];
    sprintf(se_id,"0x%d",id);

    strcpy(name,"Name");

    strcpy(opmonitor,"OPMonitor");
    strcpy(action,"Action");

    strcpy(para,"Parameter");
    strcpy(channel,"Channel");
    strcpy(combin,"CombinMode");
    strcpy(stream,"StreamType");
    strcpy(tran,"TransMode");

    strcpy(session,"SessionID");

    link_two_1(channel, "0");
    link_two(combin, "NONE");
    link_two(stream, "Main");
    link_two(tran, "TCP");

    link_three(channel, combin);
    link_three(channel, stream);
    link_three(channel, tran);

    link_four_1(channel);

    link_two_1(para, channel);
    link_two(action,act);

    link_three(action, para);

    link_four_1(action);

    link_two_1(opmonitor, action);
    link_two(name,"OPMonitor");
    link_two(session,se_id);

    link_three(name, opmonitor);
    link_three(name, session);

    link_four(name);

    get_control_flow(buf, id, 0x00, 0x00,  0x00, mes_id, strlen(name));
    strcpy(buf+sizeof(struct _control_flow),name);

#ifdef PRINTABLE
    printf("name buf \n");

    for(uint i=0;i<(sizeof(struct _control_flow) + strlen(name));i++)
    {
        printf("%c", buf[i]);
    }
    printf("\n");
#endif

    socket_write (fd, buf, sizeof(struct _control_flow) + strlen(name));

}
