#include "socket.h"

Socket::Socket(QObject *parent) : QObject(parent)
{
    udp_socket = new QUdpSocket();
    connect(udp_socket,SIGNAL(readyRead()),this,SLOT(read_data()));

    mSocket_data = new QUdpSocket();
    mSocket_data->bind(6205);
    connect(mSocket_data,SIGNAL(readyRead()),this,SLOT(read_data1()));

    startTimer(1000 * 20);

    flag = 0;
}

void Socket::wifi_aplist()
{
    send_data("wifi:aplist",6666);
}

void Socket::wifi_connect_route(QString name, QString key)
{
    send_data(QString("wifi:route:%1,WPA2PSK,AES,%2,DHCP").arg(name).arg(key),6666);
}

void Socket::wifi_create_ap(QString name, QString key, QString gateway, QString mask)
{
    if(gateway.isEmpty()){
        gateway = "192.168.5.1";
    }
    if(mask.isEmpty()){
        mask = "255.255.255.0";
    }
    send_data(QString("wifi:ap:%1,%2,%3,%4").arg(name).arg(key).arg(gateway).arg(mask),6666);
}

void Socket::open_camera()
{
    qDebug()<<"open_camera";
    char open_code[5] ={0xeb,0x90,0xeb,0x90,0x55};       //向camera程序发送通知信号,
    QByteArray str(open_code);
    qint64 len = udp_socket->writeDatagram(str,QHostAddress::LocalHost,6200);
    if(len<0){
        qDebug()<<"Socket error : "<<udp_socket->errorString();
    }
}

void Socket::close_camera()
{
    qDebug()<<"close_camera";
    send_data(QString::number(0xeb90eb90aa,16),6200);
}

void Socket::send_data(QString str, int port)
{
    qint64 len = udp_socket->writeDatagram(str.toUtf8(),QHostAddress::LocalHost,port);
    qDebug()<<"Socket writeDatagram : "<<str.toUtf8();
    if(len<0){
        qDebug()<<"Socket error : "<<udp_socket->errorString();
    }
}

void Socket::read_data()
{
    qDebug()<<"get Socket data";
    QByteArray array;
    QHostAddress address;
    quint16 port;
    array.resize(udp_socket->bytesAvailable());//根据可读数据来设置空间大小
    int len = udp_socket->readDatagram(array.data(),array.size(),&address,&port); //读取数据
    qDebug()<<"recv len = "<<len  << "\taddress: "<< address << "\tport: "<< port << "\ndata: "<<array.data();
}
#include <QTime>
//接收摄像头数据,并组装发送
//数据使用UDPSocket侦听6205端口
//一个完整的包(包含一帧图像)可能由不止一次UDP报文组成
//UDP报文最大为8192个数据,所以认为如果数据不满8192个即是尾包
//算法不完美,但出错概率较低
void Socket::read_data1()
{


#if 0

    QByteArray array;
    QHostAddress address;
    quint16 port;
    array.resize(mSocket_data->bytesAvailable());//根据可读数据来设置空间大小
    int len = mSocket_data->readDatagram(array.data(),array.size(),&address,&port); //读取数据
    //    qDebug()<<"recv len = "<<len  << "\taddress: "<< address << "\tport: "<< port /*<< "\ndata: "<<array.data()*/;
    buf_data.append(array);
    if(len != 8192){
        QByteArray a = buf_data;

        emit s_camera_packet(a);         //发送一个数据包(包含完整的一帧)
        buf_data.clear();           //这里可能会有问题,需要测试,如果不行,需要做显式拷贝
    }
#endif

//    qDebug()<<"get Socket data, flag=" << flag;

    QByteArray array;
    QHostAddress address;
    quint16 port;
    array.resize(mSocket_data->bytesAvailable());//根据可读数据来设置空间大小
    int len = mSocket_data->readDatagram(array.data(),array.size(),&address,&port); //读取数据


//    qDebug()<<"tcp begin:"<<QTime::currentTime().toString("HH-mm-ss-zzz");
    switch (flag) {
    case 0:
        buf_data0.append(array);
        if(len != 8192){
            emit s_camera_packet(buf_data0, 0);         //发送一个数据包(包含完整的一帧)

            flag ++ ;
        }
        break;
    case 1:
        buf_data1.append(array);
        if(len != 8192){
            emit s_camera_packet(buf_data1, 1);         //发送一个数据包(包含完整的一帧)

            flag ++ ;
        }
        break;
    case 2:
        buf_data2.append(array);
        if(len != 8192){
            emit s_camera_packet(buf_data2, 2);         //发送一个数据包(包含完整的一帧)

            flag = 0 ;
        }
        break;
    default:
        break;
    }



    //    qDebug()<<"recv len = "<<len  << "\taddress: "<< address << "\tport: "<< port /*<< "\ndata: "<<array.data()*/;
}

void Socket::read_done(int f)
{
    switch (f) {
    case 0:
        buf_data0.clear();
//        qDebug()<<" buf_data0.clear()";
        break;
    case 1:
        buf_data1.clear();
//        qDebug()<<" buf_data1.clear()";
        break;
    case 2:
        buf_data2.clear();
//        qDebug()<<" buf_data2.clear()";
        break;
    default:
        break;
    }
}



















