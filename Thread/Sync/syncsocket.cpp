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
    QTimer::singleShot(0, this, SLOT(init()));
}

SyncSocket::~SyncSocket()
{
}

void SyncSocket::init()
{
    //    recv_times_count = 0;
    connect_num = 0;
    recv_buf = new char[300];

    ip_addr = "192.168." + QString::number(sqlcfg->get_global()->mac_code) + ".23";

    sync_socket = new QTcpSocket;
    connect(sync_socket, SIGNAL(readyRead()), this, SLOT(deal_read()));         //读取数据

    sync_compute = new SyncCompute;
    connect(sync_compute, SIGNAL(send_msg(char *,int)), this, SLOT(deal_write(char *, int)));       //发送数据
    connect(sync_compute, SIGNAL(get_reply()), this, SLOT(deal_reply()));                           //保活机制
    connect(sync_compute, SIGNAL(get_sync()), this, SLOT(deal_sync()), Qt::DirectConnection);      //发送同步回复信息

    connect(sync_compute, SIGNAL(send_gps_info(GPSInfo*)), this, SIGNAL(send_gps_info(GPSInfo*)));    //发送GPS信息
    connect(sync_compute, SIGNAL(send_freq(float)), this, SIGNAL(send_freq(float)));    //发送同步频率
    connect(sync_compute, SIGNAL(send_sync(qint64,qint64)), this, SIGNAL(send_sync(qint64,qint64)), Qt::DirectConnection);      //发送同步时间


    sync_socket->connectToHost(ip_addr, 6991);

    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(deal_sync()));
    timer->setInterval(100);
    //    timer->start();

    timer_reconnect = new QTimer;
    connect(timer_reconnect, SIGNAL(timeout()), this, SLOT(re_connect()));
    timer_reconnect->setInterval(5000);                 //5秒重连
    timer_reconnect->start();
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

/**************************************************************************
 * 重连机制
 * 1.如果30秒未收到数据，则断开socket，尝试重连
 * 2.没有额外的保活机制
 * ***********************************************************************/
void SyncSocket::re_connect()
{
    //    qDebug()<<sync_socket->state();
    //    qDebug()<<"connect_num:"<<connect_num;

    connect_num++;
    if(connect_num >= 6 && sync_socket->state() == QAbstractSocket::ConnectedState){        //30s未收到数据断开连接
        emit update_status(tr("未连接"));
        sync_socket->disconnectFromHost();
        connect_num = 0;
        return;
    }
    if (sync_socket->state() == QAbstractSocket::UnconnectedState){
        sync_socket->connectToHost(ip_addr, 6991);         //断线重连
//        qDebug()<<ip_addr;

        //        qDebug()<<"sync socket re connect";
        emit update_status(tr("未连接"));
    }
}

void SyncSocket::deal_read()
{
//    sync_compute->show_time("read");

    //将接收内容存储到字符串中
    int len = sync_socket->read(recv_buf, 300);
//    qDebug()<<"get one packet, len = "<<len;
    connect_num = 0;
    emit update_status(tr("已连接"));


#if 0
    if(recv_buf[3] == 0x60){
        float group_num = recv_buf[9] * 0x1000000 + recv_buf[10] * 0x10000 + recv_buf[11] * 0x100  + recv_buf[12];          //组号
        if(group_num == 0){
            qDebug()<<"\n===========================begin==============================";
        }
//        for (int i = 0; i < len; ++i) {
//            printf("%02X ", recv_buf[i]);
//        }
//        printf("\n");

        if(group_num < 10){              //不满10次,发送回复
//            qDebug()<<"55aa" << "\tgroup_num:"<<group_num;
            sync_socket->write("\x55\xaa\x55\xaa",4);       //回复信号
        }
        else{
            qDebug()<<"===========================end=============++=================\n";
        }
    }
#endif

    sync_compute->read_modbus_packet(recv_buf, len);
}

/***********************************************************
 * 1.发送sync_compute中需要发送的数据
 * 2.占用线程时间,增加了不确定性
 * ********************************************************/
void SyncSocket::deal_write(char *buf, int len)
{

//    qDebug()<<"write data!" << len;
    if(len > 0){
        sync_socket->write((const char *)buf, len);       //回复信号
//        for (int i = 0; i < len; ++i) {
//            printf("%02x ", buf[i]);
//        }
//        printf("\n");
    }
}

//收到保活回复
void SyncSocket::deal_reply()
{
    connect_num = 0;
}

void SyncSocket::deal_sync()
{
//    qDebug()<<"55aa";
    sync_socket->write("\x55\xaa\x55\xaa",4);       //回复信号

//    sync_compute->show_time("send");

}
