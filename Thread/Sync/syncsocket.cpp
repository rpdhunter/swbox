#include "syncsocket.h"
#include <QtDebug>
#include <QTimerEvent>

/*********************************************
 * 同步入口类,主要完成
 * 1.与同步器相连接,每5秒发送保活指令
 * 2.同步器失联后,能定时重连
 * 3.发送同步模式指令
 * ******************************************/
SyncSocket::SyncSocket(QObject *parent) : QObject(parent)
{
    recv_times_count = 0;
    connect_num = 0;
    recv_buf = new char[300];

    sync_socket = new QTcpSocket;
    connect(sync_socket, SIGNAL(readyRead()), this, SLOT(deal_read()));         //读取数据

    sync_compute = new SyncCompute;
    connect(sync_compute, SIGNAL(send_msg(char *,int)), this, SLOT(deal_write(char *, int)));       //发送数据
    connect(sync_compute, SIGNAL(get_reply()), this, SLOT(deal_reply()));                           //保活机制
    connect(sync_compute, SIGNAL(get_gps_info(GPSInfo*)), this, SIGNAL(get_gps_info(GPSInfo*)));    //发送GPS信息
    connect(sync_compute, SIGNAL(get_sync(float)), this, SIGNAL(get_sync(float)));                  //发送同步信息

    sync_socket->connectToHost("192.168.150.23", 6991);
//    this->startTimer(5000);
}

SyncSocket::~SyncSocket()
{
}

/*********************************************
 * 组装[同步源设置]报文
 * ******************************************/
void SyncSocket::set_sync_source(int source)
{
    uchar send_buf[8];

    send_buf[0] = MODBUS_BC_ADDR;        //设备地址
    send_buf[1] = MODBUS_FC_WRITE_A_REG; //功能码
    send_buf[2] = 0x00;                  //寄存器地址(高位)
    send_buf[3] = 0x62;                  //寄存器地址(低位)
    send_buf[4] = 0x00;                  //值(高位)
    send_buf[5] = source;                //值(低位)

    //装配CRC校验码
    unsigned short crc_calc = SyncCompute::modbus_crc (send_buf, 6);     //crc的前后顺序不一样
    send_buf [6] = crc_calc / 0x100;
    send_buf [7] = crc_calc % 0x100;

    deal_write((char *)send_buf, 8);
}

bool SyncSocket::is_connect()
{
    if(sync_socket != NULL && sync_socket->state() == QAbstractSocket::ConnectedState)
        return true;
    else
        return false;
}

void SyncSocket::timerEvent(QTimerEvent *)
{
    qDebug()<<sync_socket->state();
//    if(sync_socket->state() != QAbstractSocket::ConnectedState){
    if (connect_num >= 3){
        sync_socket->connectToHost("192.168.150.23", 6991);         //断线重连
        set_sync_source(sqlcfg->get_para()->sync_mode);
        emit update_status(tr("未连接"));
    }
    else{
        set_sync_source(sqlcfg->get_para()->sync_mode);         //发送同步方式,同时也是一种保活机制

        qDebug()<<"connect_num:"<<connect_num;
        if(connect_num >= 3 && connect_num%3 ==0){
            emit update_status(tr("未连接"));
//            sync_socket->disconnectFromHost();
//            sync_socket
        }
        else if(connect_num == 0){
            emit update_status(tr("已连接"));
        }
        connect_num++;
    }
}

void SyncSocket::deal_read()
{
    //将接收内容存储到字符串中
    int len = sync_socket->read(recv_buf, 300);
    qDebug()<<"get one packet, len = "<<len;

    for (int i = 0; i < len; ++i) {
        printf("%02X ", recv_buf[i]);
    }
    printf("\n");

    if(recv_buf[3] == 0x60){
        if(recv_times_count < 10){
            sync_socket->write("\x55\xaa\x55\xaa",4);       //回复信号
            recv_times_count++;
        }
        else{
            recv_times_count = 0;

            qDebug()<<"========================end===========================\n";
        }        
    }
    sync_compute->read_modbus_packet(recv_buf, len);
}

/***********************************************************
 * 1.发送sync_compute中需要发送的数据
 * 2.占用线程时间,增加了不确定性
 * ********************************************************/
void SyncSocket::deal_write(char *buf, int len)
{

    qDebug()<<"write data!" << len;
    if(len > 0){
        sync_socket->write((const char *)buf, len);       //回复信号
        for (int i = 0; i < len; ++i) {
            printf("%02x ", buf[i]);
        }
        printf("\n");
    }
}

//收到保活回复
void SyncSocket::deal_reply()
{
    connect_num = 0;
}
