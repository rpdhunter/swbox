#include "cameradata.h"

extern  Camera_DVR   camera;

CameraData::CameraData(QObject *parent) : QThread(parent)
{
    flag = 0;
    this->start();
}

CameraData::~CameraData()
{

}


void CameraData::run()
{
    char buf[8212];
    char order_buf[1000];

    int ret,len=0,total_len=0;
    media_flow  *media;

    int recv_flag=0;
    int len1=0,total_len1=0;
    control_flow  *control;

    fd_set fds;
    int maxfd;
    struct timeval timeout;

    while(1){

        FD_ZERO(&fds);
        FD_SET(camera.Data_Socket,&fds);
        FD_SET(camera.Order_Socket,&fds);

        maxfd =  (camera.Data_Socket > camera.Order_Socket ? camera.Data_Socket : camera.Order_Socket)+ 1;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        ret = select(maxfd,&fds,NULL,NULL,&timeout);

        if ((ret > 0) && (FD_ISSET(camera.Data_Socket, &fds))) {               //接受数据

            if(total_len < sizeof(struct _media_flow)){                                                                                 //先接受头部
                len = read(camera.Data_Socket, buf+total_len, sizeof(struct _media_flow)-total_len);
            }else{
                len = read(camera.Data_Socket, buf+total_len,
                           camera.Data_Data_Len - total_len + sizeof(struct _media_flow));                             //根据数据长度接受剩余数据
            }

            total_len = total_len + len;
            if(total_len == sizeof(struct _media_flow)){                                           //接受完头部
                media = (struct _media_flow *)buf;
                if((media->Head_Flag == 0xFF) && (media->Version == 0x01)  \
                        && (media->Reserve1 == 0x00) && (media->Reserve2 == 0x00)){
                    camera.Data_Message_Id= media->Message_Id;
                    camera.Data_Data_Len = media->Data_Length;
                }else{						//如果头部数据不对，则逐个寻找头部
                    memmove(buf, buf+1, total_len-1);
                    total_len = total_len -1;
                    continue;
                }
            }

            if(total_len == (sizeof(struct _media_flow) + camera.Data_Data_Len)){               			 //接收到所有数据

                switch(camera.Data_Message_Id)
                {
                case  MONITORCLAIM_REQ  :  break;                       //监控认领
                case  MONITORDATA       :
//                    printf("Data_Message_Id : %d,    Data_Data_Len : %d \n",camera.Data_Message_Id,camera.Data_Data_Len);

                    send_data(buf+sizeof(struct _media_flow), camera.Data_Data_Len);
                    break;
                }
#if 0
                for(i=0;i<(sizeof(struct _media_flow) + camera.Data_Data_Len);i++)
                {
                    printf("%02x  ", buf[i]);
                    if((i+1)%20 == 0){
                        printf("\n");
                    }
                }
                printf("\n");
#endif
                total_len = 0;
            }
        }
        else if((ret > 0) && (FD_ISSET(camera.Order_Socket, &fds))){                         //接受指令
            len1 = read(camera.Order_Socket, order_buf+total_len1, sizeof(order_buf)-total_len1);
            total_len1 = total_len1 + len1;

            if((total_len1 > sizeof(struct _media_flow)) && (recv_flag == 0)){                                                        //接受完头部
                control = (struct _control_flow *)order_buf;
                camera.Order_Message_Id = control->Message_Id;
                camera.Session_Id = control->Session_Id;
                camera.Order_Data_Len = control->Data_Length;
                printf("Order_Message_Id : %d,      Session_id : %d,     Order_Data_Len : %d \n\n",
                       camera.Order_Message_Id,camera.Session_Id,camera.Order_Data_Len);
                recv_flag = 1;
            }

            if(total_len1 >= (sizeof(struct _control_flow) + camera.Order_Data_Len)){                //接收到所有数据

                switch(camera.Order_Message_Id)
                {
                case  LOGIN_RET               :  break;                 //登录
                case  REALMONITOR_RET  :  break;			//实时监视
                case  KEEPALIVE_RET        :  break;			//包活
                }
#if 0
                for(i=0;i<(sizeof(struct _control_flow) + camera.Order_Data_Len);i++)
                {
                    printf("%02x  ", buf[i]);
                    if((i+1)%20 == 0){
                        printf("\n");
                    }
                }
                printf("\n");
#endif
                recv_flag = 0; total_len1 = 0;
            }
        }
        else{
            total_len = 0;recv_flag = 0;total_len1 = 0;
        }
    }
}

void CameraData::send_data(char *data, int len)
{
    QByteArray arrey(data, len);     //这里执行深度拷贝
    packet.append(arrey);
    if (camera.Data_Data_Len != 8192){
        emit sendOnePacket(packet);
        packet.clear();             //直接清除没问题,已通过测试
    }
}
