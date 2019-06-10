#include "camerasocket.h"
#define  PRINTABLE

CameraSocket::CameraSocket(QObject *parent) : QObject(parent)
{
    thread = new QThread;
    this->moveToThread(thread);
    thread->start();

    QTimer::singleShot(0, this, SLOT(socket_init()));
}

/********************************************************
 * 线程的初始化
 * 1.建立指令套接字和数据套接字
 * 2.启动一个计时器,用于socket重连
 * *****************************************************/
void CameraSocket::socket_init()
{
    order_socket = new QTcpSocket;
    connect(order_socket, SIGNAL(readyRead()), this, SLOT(deal_order()));

    data_socket = new QTcpSocket;
    connect(data_socket, SIGNAL(readyRead()), this, SLOT(deal_data()));

    control = new control_flow;
    control->Session_Id = 0;

    media = new media_flow;
    media->Data_Length = 0;

    socket_connect();         //连接socket

    timer = new QTimer;
    timer->setInterval(5000);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(socket_connect()));          //socket重连
}

/********************************************************
 * 与摄像头连接
 * 1.尝试登陆
 * 2.发送认领和监视请求
 * *****************************************************/
void CameraSocket::connect_camera()
{
    if(order_socket->state() == QAbstractSocket::ConnectedState){
        QByteArray order;
        order = Login_Req("admin", "tlJwpbo6");
        order_socket->write(order.data(), order.length());             //发送登录信息
        print_info(order.data(), order.length(), "Login_Req");
        order_socket->waitForReadyRead();

        order = Monitor_Claim(control->Session_Id, "Claim", MONITORCLAIM_REQ);
        data_socket->write(order.data(), order.length());             //发送data认领请求
        print_info(order.data(), order.length(), "Monitor_Claim");
        data_socket->waitForReadyRead();

        order = Monitor_Claim(control->Session_Id, "Start", REALMONITOR_REQ);
        order_socket->write(order.data(), order.length());             //发送control监视请求
        print_info(order.data(), order.length(), "Monitor_Claim");
        order_socket->waitForReadyRead();
    }
}

void CameraSocket::socket_connect()
{
    //指令套接字
    if(order_socket->state() == QAbstractSocket::UnconnectedState){     //未连接,建立连接
        order_socket->connectToHost(CAMERA_IP, 34567);
        if (!order_socket->waitForConnected(3 * 1000)){
//            qDebug()<<"camera order_socket failed!";
        }
        else{
//            qDebug()<<"camera order_socket succeeded!";
        }
    }
    else if(order_socket->state() == QAbstractSocket::ConnectedState && control->Session_Id > 0){  //已连接,保活
        QByteArray order;
        order = KeepAlive(control->Session_Id);
        print_info(order.data(), order.length(), "KeepAlive");
        order_socket->write(order.data(), order.length());             //发送登录信息
    }

    //数据套接字
    if(data_socket->state() == QAbstractSocket::UnconnectedState){      //未连接,建立连接
        data_socket->connectToHost(CAMERA_IP, 34567);
        if (!data_socket->waitForConnected(3 * 1000)){
//            qDebug()<<"camera data_socket failed!";
        }
        else{
//            qDebug()<<"camera data_socket succeeded!";
        }
    }
}

void CameraSocket::deal_order()
{
    QByteArray array = order_socket->readAll();

    qDebug()<<"get order data: len = " << array.count() << array;

    control = (struct _control_flow *)array.data();
    printf("Order_Message_Id : %d,      Session_id : %d,     Order_Data_Len : %d \n\n",
           control->Message_Id,control->Session_Id,control->Data_Length);

}

void CameraSocket::deal_data()
{
    QByteArray array = data_socket->readAll();
//    qDebug()<<"get data data: len = " << array.count()/* << array*/;

    int to_read = 0;
    while (!array.isEmpty()) {
        if(media->Data_Length < packet.length()){       //应读<已读,此种情况说明发生错误(保证to_read为正)
            media->Data_Length = 0;
            packet.clear();
            qDebug()<<"CameraSocket deal_data error!";
        }
        if(media->Data_Length == 0){
            find_media_flow(array);
        }
        to_read = media->Data_Length - packet.length();     //还需要填充的字节数
        if(array.length() >= to_read) {             //待读≥应读
            packet.append(array.mid(0,to_read));    //一部分数据将当前包填满

//            qDebug()<<"packet complete! >>>>>>>>>>>>>>>>>>>> len="<<packet.length() << endl;
            frame.append(packet);
            if(packet.length() != 8192){
                emit sendOneFrame(frame);
                qDebug()<<"frame complete! len:"<<frame.count();
                frame.clear();
            }
            packet.clear();
            media->Data_Length = 0;
            array.remove(0,to_read);                //将已读删除
        }
        else{
            packet.append(array);                   //读到的数据全部填充包
            array.clear();
        }
    }
}

/********************************************************
 * 寻找报文头
 * 1.找到,去掉头部及之前的数据,返回true
 * 2.未找到,清空list,返回false
 * *****************************************************/
bool CameraSocket::find_media_flow(QByteArray &list)
{
    _media_flow *temp_media;
    for (uint i = 0; i < list.count()-sizeof(struct _media_flow); ++i) {
        temp_media = (struct _media_flow *)(list.data()+i);
        if((temp_media->Head_Flag == 0xFF) && (temp_media->Version == 0x01)              //校验头部4数据
                && (temp_media->Reserve1 == 0x00) && (temp_media->Reserve2 == 0x00) && temp_media->Data_Length <= 8192){
            memcpy(media, temp_media, sizeof(struct _media_flow));              //保存头部
            list.remove(0, i+sizeof(struct _media_flow));    //去掉头部及之前的数据
//            qDebug()<<"find media_flow! <<<<<<<<<<<<<<<<<<<<< len ="<<media->Data_Length << "\t i =" <<i;
//            qDebug()<<"Session_Id:"<<media->Session_Id << "\t Sequence_Number:" <<media->Sequence_Number
//                   <<"Channel:"<<(int)media->Channel << "\t End_Flag:" <<(int)media->End_Flag
//                     <<"Message_Id:"<<media->Message_Id;
//            switch(media->Message_Id)
//            {
//            case  MONITORCLAIM_REQ  :  break;                       //监控认领
//            case  MONITORDATA       :
//                printf("Data_Message_Id : %d,    Data_Data_Len : %d \n",media->Message_Id,media->Data_Length);

//                //        send_data(buf+sizeof(struct _media_flow), camera.Data_Data_Len);
//                break;
//            }
            return true;
        }
    }
    list.clear();
    return false;
}

QByteArray CameraSocket::Login_Req(QString user, QString password)
{
    QStringList list;
    list << phrase("EncryptType", "MD5");
    list << phrase("LoginType", "DVRIP-Web");
    list << phrase("PassWord", password);
    list << phrase("UserName", user);

    QByteArray data =  statement(list).toLocal8Bit();
    return add_control_flow(data, 0, LOGIN_REQ);
}

QByteArray CameraSocket::KeepAlive(int id)
{
    QStringList list;
    list << phrase("Name", "KeepAlive");
    list << phrase("SessionID", QString("0x%1").arg(id,8,16,QChar('0')));

    QByteArray data = statement(list).toLocal8Bit();
    return add_control_flow(data, id, KEEPALIVE_REQ);
}

QByteArray CameraSocket::Monitor_Claim(int id, QString act, unsigned short mes_id)
{
    QStringList list1;
    list1 << phrase("Channel", 0);
    list1 << phrase("CombinMode", "NONE");
    list1 << phrase("StreamType", "Main");
    list1 << phrase("TransMode", "TCP");
    QString statement1 = statement(list1);

    QStringList list2;
    list2 << phrase("Action", act);
    list2 << phrase("Parameter", statement1);
    QString statement2 = statement(list2);

    QStringList list3;
    list3 << phrase("Name", "OPMonitor");
    list3 << phrase("OPMonitor", statement2);
    list3 << phrase("SessionID", QString("0x%1").arg(id,8,16,QChar('0')));

    QByteArray data = statement(list3).toLocal8Bit();
    return add_control_flow(data, id, mes_id);
}

QString CameraSocket::phrase(QString s1, QString s2)
{
    if(s2.at(0) == "{")
        return QString("\"%1\" : %2").arg(s1).arg(s2);
    else
        return QString("\"%1\" : \"%2\"").arg(s1).arg(s2);
}

QString CameraSocket::phrase(QString s1, int s2)
{
    return QString("\"%1\" : %2").arg(s1).arg(s2);
}

QString CameraSocket::statement(QStringList list)
{
    QString s = list.join(", ");
    return "{ " + s + " }";
}

QByteArray CameraSocket::add_control_flow(QByteArray data, int id, unsigned short mes_id)
{
    control_flow  control;
    control.Head_Flag = 0xFF;
    control.Version = 0x00;
    control.Reserve1 = 0x00;
    control.Reserve2 = 0x00;
    control.Session_Id = id;
    control.Sequence_Number = 0x00;
    control.Total_Packet = 0x00;
    control.Cur_Packet = 0x00;
    control.Message_Id = mes_id;
    control.Data_Length = data.length();

    QByteArray head;
    head.resize(sizeof(struct _control_flow));
    memcpy(head.data(), &control, sizeof(struct _control_flow));
    head.append(data);
    return head;
}

void CameraSocket::print_info(char *buf, int len, QString name)
{
#ifdef PRINTABLE
    qDebug()<<name;
    for(int i=0;i<len;i++)
    {
        printf("%c", buf[i]);
    }
    printf("\n");
#endif
}



