#include "camerasocket1.h"

//#define  PRINTABLE

Camera_DVR   camera = {0};

CameraSocket1::CameraSocket1(QObject *parent) : QObject(parent)
{
    timer_alive = new QTimer();
    timer_alive->setInterval(5000);   //10秒1跳
    timer_alive->start();

    connect(timer_alive, SIGNAL(timeout()), this, SLOT(keep_alive()) );

    connect_camera();
}

CameraSocket1::~CameraSocket1()
{
}

void CameraSocket1::connect_camera()
{
    int num;
    pthread_t  order_th, data_th;

    camera.Order_Socket = connect_socket();                  //指令socket
    if(camera.Order_Socket  < 0){
        printf("order_socket is error!\n");
        return;
    }
    printf("order_socket is ok!\n");

    camera.Data_Socket = connect_socket();                  //数据socket
    if(camera.Data_Socket  <  0){
        printf("data_socket is error!\n");
        close(camera.Order_Socket);
        return;
    }
    printf("data_socket is ok!\n");

    data = new CameraData(this);                           //创建接受socket
    connect(data, SIGNAL(sendOnePacket(QByteArray)), this, SIGNAL(sendOnePacket(QByteArray)));

    camera.Session_Id = 0;
    num = 0;
    while((camera.Session_Id == 0) && (num < 3)){
        qDebug()<<"Login_Req:"<< num+1;
        Login_Req(camera.Order_Socket, "admin", "tlJwpbo6");              //发送登录信息
        sleep(2);
        num ++;
    }
    if(num >= 3){														  //如果三次都没登录上，则退出
        close(camera.Order_Socket);
        close(camera.Data_Socket);
        pthread_cancel (order_th);
        pthread_cancel (data_th);
        printf("can not login in  camera!");
        return;
    }

    Monitor_Claim(camera.Data_Socket, camera.Session_Id,"Claim",MONITORCLAIM_REQ) ;      //发送认领请求
    sleep(1);
    Monitor_Claim(camera.Order_Socket, camera.Session_Id,"Start",REALMONITOR_REQ) ;        //发送监视请求
    sleep(1);
}

void CameraSocket1::keep_alive()
{
    KeepAlive(camera.Order_Socket, camera.Session_Id);  //定期发送包活机制
}

void CameraSocket1::link_one(char *src)                             // "ABCD"
{
    char *p;
    char a[200]="\"";
    p = strcat(strcat(a,src),"\"");
    strcpy(src,p);
}

void  CameraSocket1::link_two(char *src1, char *src2)        // "ABCD : CDEF"    注意中间的空格
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

void  CameraSocket1::link_two_1(char *src1, char *src2)        // "ABCD" : CDEF    注意中间的空格
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

void  CameraSocket1::link_three(char *src1,char *src2)    //"ABCD : CDEF", "ABCD : CDEF"       ,号和空格
{
    char *p;
    p = strcat(strcat(src1,", "),src2);
    strcpy(src1,p);
}

void CameraSocket1::link_four(char *src)                           //{ "ABCD : CDEF", "ABCD : CDEF" }
{
    char p[200];
    strcpy(p,"{ ");
    strcat(p,src);
    strcat(p," }\n");
    strcpy(src,p);
}
void CameraSocket1::link_four_1(char *src)                           //{ "ABCD : CDEF", "ABCD : CDEF" }
{
    char p[200];
    strcpy(p,"{ ");
    strcat(p,src);
    strcat(p," }");
    strcpy(src,p);
}

void CameraSocket1::get_control_flow(char *buf, int id, int seq_num, char tol_pck, char cur_pck, unsigned short mes_id, int data_len)
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




int CameraSocket1::connect_socket()
{
    int fd;
    struct sockaddr_in s_add;

    fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd < 0){
        printf("create socket  is  error !");
        return -1;
    }

    s_add.sin_addr.s_addr=inet_addr(CAMERA_IP);
    s_add.sin_family=AF_INET;       //IPv4 Internet协议
    s_add.sin_port=htons(34567);

    if(-1 == ::connect(fd, (struct sockaddr *)(&s_add),sizeof(struct sockaddr)))
    {
        printf("connect fail!");
        return -1;
    }
    return fd;
}

void CameraSocket1::Login_Req(int fd, char *user, char *password)
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
    qDebug()<< "Login_Req len = "<<(sizeof(struct _control_flow) + strlen(encry));

    for(uint i=0;i<(sizeof(struct _control_flow) + strlen(encry));i++)
    {
        printf("%c", buf[i]);
    }
    printf("\n");
#endif

    socket_write (fd, buf, sizeof(struct _control_flow) + strlen(encry));


#if 0
    printf("%s \n",encry);
    for(i=0;i<(sizeof(struct _control_flow) + strlen(encry));i++)
    {
        printf("%02x  ", buf[i]);
        if((i+1)%8 == 0){
            printf("\n");
        }
    }
    printf("\n num = %d \n",strlen(encry));
#endif
}

int  CameraSocket1::socket_write (int fd, char * buf, int len)
{
    int write_len, offset;
    for (offset = 0; len > 0; len -= write_len) {
        write_len = write(fd, buf + offset, len);
        if (write_len < 0) {
            return -1;
        }
        if (write_len < len) {
            offset += write_len;
        }
    }
    return 0;
}
void  CameraSocket1::Monitor_Claim(int fd, int id, char *act,unsigned short mes_id)       //监视认领请求和监视请求
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


void  CameraSocket1::KeepAlive(int fd, int id)       //监视认领请求和监视请求
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
