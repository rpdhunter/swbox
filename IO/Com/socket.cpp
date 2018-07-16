#include "socket.h"

Socket::Socket(QObject *parent) : QObject(parent)
{
    mSocket = new QUdpSocket();
    connect(mSocket,SIGNAL(readyRead()),this,SLOT(read_data()));
}

void Socket::wifi_set_baud(int b)
{
    send_data(QString("wifi:baud:%1").arg(b) );
}

void Socket::wifi_set_mode(WIFI_MODE mode)
{
    switch (mode) {
    case WIFI_AP:
        send_data("wifi:apsta:AP");
        break;
    case WIFI_STA:
        send_data("wifi:apsta:STA");
        break;
    case WIFI_APSTA:
        send_data("wifi:apsta:APSTA");
        break;
    default:
        break;
    }
}

void Socket::wifi_aplist()
{
    send_data("wifi:aplist");
}

void Socket::wifi_connect_route(QString name, QString key)
{
    send_data(QString("wifi:route:%1,WPA2PSK,AES,%2,DHCP").arg(name).arg(key));
}

void Socket::wifi_create_ap(QString name, QString key, QString gateway, QString mask)
{
    if(gateway.isEmpty()){
        gateway = "192.168.5.1";
    }
    if(mask.isEmpty()){
        mask = "255.255.255.0";
    }
    send_data(QString("wifi:ap:%1,%2,%3,%4").arg(name).arg(key).arg(gateway).arg(mask));
}

void Socket::telnet_start(int baud, int port)
{
    send_data(QString("telnet:start:%1,TCP,server,%2").arg(baud).arg(port));
}

void Socket::telnet_stop()
{
    send_data("telnet:stop");
}

void Socket::ftp_start(int baud, int port)
{
    send_data(QString("ftp:start:%1,TCP,server,%2").arg(baud).arg(port));
}

void Socket::ftp_stop()
{
    send_data("ftp:stop");
}

void Socket::_104_start(int baud, int port)
{
    send_data(QString("104:start:%1,TCP,server,%2").arg(baud).arg(port));
}

void Socket::_104_stop()
{
    send_data("104:stop");
}

void Socket::send_data(QString str)
{
    //单播
//     qint64 len = mSocket->writeDatagram(ui->textEdit->toPlainText().toUtf8(),QHostAddress("192.168.20.17"),6666);

//    qint64 len = mSocket->writeDatagram(ui->textEdit->toPlainText().toUtf8(),QHostAddress::LocalHost,6666);

    //组播ip地址范围：224.0.0.0-239.255.255.255
    //qint64 len = mSocket->writeDatagram(ui->textEdit->toPlainText().toUtf8(),QHostAddress("224.0.0.100"),6666);
    //广播
//    qint64 len = mSocket->writeDatagram(ui->textEdit->toPlainText().toUtf8(),QHostAddress::Broadcast,6666);

//    qint64 len = mSocket->writeDatagram("222",QHostAddress::Broadcast,6666);

    qint64 len = mSocket->writeDatagram(str.toUtf8(),QHostAddress::LocalHost,6666);
    qDebug()<<"Socket writeDatagram : "<<str.toUtf8();
    if(len<0){
        qDebug()<<"Socket error : "<<mSocket->errorString();
    }
}

void Socket::read_data()
{
//    qDebug()<<"get Socket data";
    QByteArray array;
    QHostAddress address;
    quint16 port;
    array.resize(mSocket->bytesAvailable());//根据可读数据来设置空间大小
    int len = mSocket->readDatagram(array.data(),array.size(),&address,&port); //读取数据
    qDebug()<<"recv len = "<<len  << "\taddress: "<< address << "\tport: "<< port << "\ndata: "<<array.data();


    QString str(array.data());
    qDebug()<<str;
    QStringList list;
    list = str.split(",");
    if(list.first() == "AP"){                       //累计APList
        aplist.append(str);
    }
    else if(list.first() == "APLISTEND"){           //发送APList
        emit s_wifi_aplist(true,aplist);
        aplist.clear();
    }
    else if(list.first() == "wifi_baud"){           //验证波特率
        bool b = list.at(1) == "OK" ? true : false;
        emit s_wifi_set_baud(b);
    }
    else if(list.first() == "wifi_apsta"){           //验证wifi模式
        bool b = list.at(1) == "OK" ? true : false;
        emit s_wifi_set_mode(b);
    }
    else if(list.first() == "wifi_route"){           //验证连接路由
        bool b = list.at(1) == "OK" ? true : false;
        list.removeFirst();
        list.removeFirst();
        s_wifi_connect_route(b,list);
//        if(list.at(1) == "password_is_error"){
//            qDebug()<<"password_is_error";
//        }
    }
    else if(list.first() == "wifi_ap"){
        bool b = list.at(1) == "OK" ? true : false;
        emit s_wifi_create_ap(b);
    }
}



















