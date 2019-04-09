#include "modbussync.h"

//串口相关的头文件
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <QtNetwork>
#include <QSerialPortInfo>
#include <QSerialPort>

//#include "IO/Com/modbus/gpio_oper.h"
//#include "IO/Com/modbus/uart_oper.c"
#include "IO/Com/rdb/rdb.h"
//#include "IO/Com/rdb/point_table.h"
#include <QTime>
#include <sys/time.h>    // for gettimeofday()
#include "Gui/Common/common.h"
#include "Algorithm/compute.h"
#include "Gui/Common/wifi.h"

struct sockaddr_in addr;

ModbusSync::ModbusSync(QObject *parent, G_PARA *g_data) : QThread(parent)
{
    int i;

    is_connect = false;

    data = g_data;
    sql_para = new SQL_PARA;
    data_stand = new unsigned short[md_wr_reg_max];

    for (i = 0; i < md_wr_reg_max; ++i) {
        data_stand [i] = 0;
    }
//    qDebug()<<"modbus start!";

    recv_times_count = 0;

    pd_dev.dev_addr = MODBUS_BC_ADDR;
    pd_dev.recv_len = 0;
//    this->start(QThread::TimeCriticalPriority);
}

void ModbusSync::set_ipaddr(QString ip)
{
    int i,j=0;
    /*    for(i=ip.length();i>0;i--)
    {
        QChar ch = ip.at(i-1);
        if(ch == '.'){
            ipaddr = ip.left(i);
            ipaddr = ipaddr + "23";
            break;
        }
    } */
    ipaddr = "192.168.150.23";
    qDebug()<<"ip : "<<ipaddr;
}

void ModbusSync::close_connect_wifi()
{
    is_connect = false;
}

void ModbusSync::set_sync_source(int source)
{
    if(pd_dev.connect_flag == 0)
        return;

    pd_dev.send_buf[0] = 0xFF;                        //设备地址不一样
    pd_dev.send_buf[1] = MODBUS_FC_WRITE_A_REG;
    pd_dev.send_buf[2] = 0x00;
    pd_dev.send_buf[3] = 0x62;
    pd_dev.send_buf[4] = 0x00;
    switch (source) {
    case sync_clamp:
        pd_dev.send_buf[5] = 0x01;
        break;
    case sync_light:
        pd_dev.send_buf[5] = 0x02;
        break;
    case sync_vac220_110:
        pd_dev.send_buf[5] = 0x03;
        break;
    default:
        pd_dev.send_buf[5] = 0x00;
        break;
    }

    //装配CRC校验码
    unsigned short crc_calc = modbus_crc (pd_dev.send_buf, 6);     //crc的前后顺序不一样
    pd_dev.send_buf [6] = crc_calc / 0x100;
    pd_dev.send_buf [7] = crc_calc % 0x100;
    pd_dev.send_len = 8;
    modbus_send_msg(&pd_dev);

    qDebug()<<"set source completed!";
}

ModbusSync::~ModbusSync()
{
}

void ModbusSync::init_socket(int socket)
{
    pd_dev.socket_fd = socket;
}
void ModbusSync::run()
{
    int len, ret, old_socket;
    unsigned char recv_buf[300];

    fd_set fds;
    int maxfd;
    struct timeval timeout;

    pd_dev.connect_flag = 0;

    while(1)
    {
        old_socket = socket(AF_INET, SOCK_STREAM, 0);       //建立一个基于IPv4 Internet协议的TCP socket
        if(old_socket < 0) {
            qDebug()<<"socket is error";
            sleep(3);
            pd_dev.connect_flag = 0;
            continue;
        }

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));   // 初始化服务器地址
        server_addr.sin_family = AF_INET;	// IPv4
        server_addr.sin_port = htons(6991);	// 端口
        server_addr.sin_addr.s_addr = inet_addr("192.168.150.23");	// ip

        int err_log = ::connect(old_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(err_log < 0){
            qDebug()<<"connect is error";
            sleep(3);
            pd_dev.connect_flag = 0;
            continue;
        }

        is_connect = true;
        re_connect_counter = 0;     //重连计数器清零

        pd_dev.socket_fd = old_socket;
        qDebug()<<"modbus is connected!";

        pd_dev.connect_flag = 1;

        while (1) {
            if(!is_connect)
//            set_sync_source(sync_vac220_110);       //设置同步源
//            set_sync_source(sync_light);       //设置同步源
            if(is_connect == false){
                close(old_socket);
                pd_dev.connect_flag = 0;
                break;
            }

            FD_ZERO(&fds);
            FD_SET(old_socket,&fds);
            maxfd = old_socket + 1;
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;

            ret = select(maxfd,&fds,NULL,NULL,&timeout);

            if  ((ret > 0) && (FD_ISSET(old_socket, &fds))){
                re_connect_counter = 0;     //重连计数器清零
                len = recv(old_socket,recv_buf,sizeof(recv_buf),0);
                if(len <= 0){
                    close(old_socket);
                    pd_dev.connect_flag = 0;
                    is_connect = false;
                    break;
                }

                double tel_delay = 0.100 * (recv_buf[13] * 0x1000000 + recv_buf[14] * 0x10000 + recv_buf[15] * 0x100  + recv_buf[16]); //延迟(ms)
                qDebug()<<"\nrecv_buf len ="<<len<<"\tdelay ="<<tel_delay;

                if(recv_buf[3] == 0x60){
                    if(recv_times_count < 10){
                        send(pd_dev.socket_fd,"\x55\xaa\x55\xaa",4,0);        //回复信号
                        recv_times_count++;
//                        qDebug()<<"recv and answer: \t"<<QTime::currentTime();
                    }
                    else{
                        qDebug()<<"recv and deal: \t"<<QTime::currentTime();
                        recv_times_count = 0;

                    }

                }
                modbus_com_recv (&pd_dev, recv_buf, len);           //处理

            }
            else{
                re_connect_counter++;
                if(re_connect_counter == 15){           //60s跳出小循环,socket开始重连
                    is_connect = false;
                }
            }
        }
    }
}


unsigned short ModbusSync::modbus_crc(unsigned char *buf, unsigned char length)
{
    unsigned short val;
    unsigned short poly = 0xa001;
    unsigned char i, j;

    val = 0xffff;
    for (i = 0; i < length; i++) {
        val ^= buf [i];
        for (j = 0; j < 8; j++){
            if ((val & 0x0001) > 0) {
                val >>= 1;
                val ^= poly;
            }
            else {
                val >>= 1;
            }
        }
    }

    return val;
}

int ModbusSync::modbus_com_clr_to(ModbusSync::modbus_dev_t *ndp)
{
    if (ndp->recv_to_cnt) {
        ndp->recv_to_cnt = 0;
    }
    if (ndp->recv_to_flag) {
        ndp->recv_to_flag = 0;
    }

    return 0;
}

int ModbusSync::modbus_com_recv(ModbusSync::modbus_dev_t *ndp, unsigned char *buf, int len)
{
    int i;

    modbus_com_clr_to (ndp);

    for (i = 0; i < len; i++) {
        ndp->recv_buf [ndp->recv_len++] = buf [i];
        if (ndp->recv_len == 1) {
            /* 装置地址为报文头 */
            if ((ndp->recv_buf [0] != ndp->dev_addr) &&
                    (ndp->recv_buf [0] != MODBUS_BC_ADDR)) {
                ndp->recv_len = 0;
            }
        }
    }
    modbus_deal_msg (ndp);
    return 0;
}

int ModbusSync::modbus_com_recv_to(ModbusSync::modbus_dev_t *ndp)
{
    if (!ndp->recv_to_flag) {
        if (ndp->recv_to_cnt++ > MODBUS_RECV_TO_CNT) {
            ndp->recv_to_flag = 1;
        }
    }

    if (ndp->recv_to_flag && ndp->recv_len > 0) {
        modbus_deal_msg (ndp);
        modbus_com_clr_to (ndp);
    }

    return 0;
}

int ModbusSync::socket_supper_udp (int fd, unsigned char * buf, int len)
{
    socklen_t sock_len;
    int write_len, offset;

    sock_len = sizeof(struct sockaddr_in);

    for (offset = 0; len > 0; len -= write_len) {
        write_len = sendto(fd, buf + offset, len, 0, (struct sockaddr *)&addr, sock_len);
        if (write_len < 0) {
            return -1;
        }
        if (write_len < len) {
            offset += write_len;
        }
    }
    return 0;
}

int ModbusSync::socket_supper_tcp(int fd, unsigned char * buf, int len)
{
    socklen_t sock_len;
    int write_len, offset;

    sock_len = sizeof(struct sockaddr_in);

    for (offset = 0; len > 0; len -= write_len) {
        //      write_len = sendto(fd, buf + offset, len, 0, (struct sockaddr *)&addr, sock_len);
        write_len = send(fd, buf + offset, len, 0);
        if (write_len < 0) {
            return -1;
        }
        if (write_len < len) {
            offset += write_len;
        }
    }
    return 0;
}

int ModbusSync::start_measurement(ModbusSync::modbus_dev_t *ndp)
{
    unsigned short start = (ndp->recv_buf[7] << 8) + ndp->recv_buf[8];
    if(start == 0x0001)
    {
        //     qDebug()<<"start mesasurement";
        //     qDebug("measurement timer is %d ",((ndp->recv_buf[9]<<8)+ndp->recv_buf[10]));
        return 0;
    }
    if(start == 0x0000)
    {
        //    qDebug()<<"stop measurement";
        return 0;
    }
    return -1;
}

int ModbusSync::modbus_send_msg(ModbusSync::modbus_dev_t *ndp)
{

    if (ndp->send_len > 0) {

        // socket_supper_udp (ndp->com_fd, ndp->send_buf, ndp->send_len);
        socket_supper_tcp(ndp->socket_fd, ndp->send_buf, ndp->send_len);

        ndp->send_len = 0;
    }

    return 0;
}

int ModbusSync::modbus_deal_msg(ModbusSync::modbus_dev_t *ndp)
{
    unsigned char func_code;
    int ret;

    if (ndp->recv_buf [0] != ndp->dev_addr &&
            ndp->recv_buf [0] != MODBUS_BC_ADDR) {
        /* 判地址 */
        ndp->recv_len = 0;
        return -1;
    }

    if (ndp->recv_len < 8) {
        /* 判报文长度 */
        ndp->recv_len = 0;
        return -1;
    }

    /* 功能码 */
    func_code = ndp->recv_buf [1];

    switch(func_code) {
    case MODBUS_FC_READ_REG:
        ret = modbus_deal_read_reg (ndp);     //读寄存器
        break;
    case MODBUS_FC_WRITE_A_REG:
        ret = modbus_deal_write_a_reg (ndp);  //写一个寄存器
        break;
    case MODBUS_FC_WRITE_MORE_REG:            //写多个寄存器
        ret = modbus_deal_write_more_reg (ndp);
        break;
    default:
        ret = -1;
        break;
    }

    /* clear recv buf */
    ndp->recv_len = 0;

    /* send message */
    if (ndp->send_len > 0) {
        //     modbus_send_msg (ndp);
    }

    return ret;
}

int ModbusSync::modbus_deal_read_reg(ModbusSync::modbus_dev_t *ndp)
{
    unsigned short crc_recv, crc_calc;
    unsigned short start_add, reg_count, reg_val;
    int i;

    if (ndp->recv_len != 8) {   //报文长度必须为8
        return -1;
    }

    //crc校验
    crc_recv = (ndp->recv_buf [7] << 8) + ndp->recv_buf [6];   //高位在前      //crc的前后顺序不一样
    crc_calc = modbus_crc (ndp->recv_buf, 6);
    if (crc_calc != crc_recv) {
        printf ("crc error! %04x, %04x\n", crc_recv, crc_calc);
        return -1;
    }

    start_add = (ndp->recv_buf [2] << 8) + ndp->recv_buf [3]; //起始地址
    reg_count = (ndp->recv_buf [4] << 8) + ndp->recv_buf [5]; //寄存器数量
    if (reg_count < 1 || reg_count > MODBUS_MAX_REG_CNT) {
        printf ("reg count error\n");
        return -1;
    }

    // check start address
    if ((start_add + reg_count) > md_rd_reg_max) {
        printf ("reg value or count error\n");
        return -1;
    }

    //装配发送的报文
    ndp->send_len = 5 + (reg_count << 1);       //长度为5+2N
    ndp->send_buf[0] = pd_dev.dev_addr;                        //设备地址不一样
    ndp->send_buf[1] = MODBUS_FC_READ_REG;
    ndp->send_buf[2] = (reg_count << 1);

    transData();        //得到设备数据

    for (i = 0; i < reg_count; i++) {

        reg_val = data_stand[start_add + i];

        //    qDebug()<<"val="<<reg_val;
        ndp->send_buf [3 + (i << 1)] = reg_val >> 8;
        ndp->send_buf [4 + (i << 1)] = reg_val & 0xff;
    }

    //装配CRC校验码
    crc_calc = modbus_crc (ndp->send_buf, 2 * reg_count + 3);     //crc的前后顺序不一样
    ndp->send_buf [2 * reg_count + 4] = crc_calc / 0x100;
    ndp->send_buf [2 * reg_count + 3] = crc_calc % 0x100;

    return 0;
}

int ModbusSync::modbus_deal_write_a_reg(ModbusSync::modbus_dev_t *ndp)
{
    unsigned short crc_recv, crc_calc;
    unsigned short reg_add, val;
    int ret;

    if (ndp->recv_len != 8) {   //报文长度必须为8
        return -1;
    }

    //crc校验

    crc_recv = (ndp->recv_buf[7] << 8)+ndp->recv_buf[6];   //高位在前
    crc_calc = modbus_crc(ndp->recv_buf,6);
    if (crc_calc != crc_recv) {
        printf ("crc error! %04x, %04x\n", crc_recv, crc_calc);
        return -1;
    }

    reg_add = (ndp->recv_buf[2] << 8)+ndp->recv_buf[3]; //寄存器地址
    val = (ndp->recv_buf[4] << 8)+ndp->recv_buf[5]; //待赋给寄存器的数值

    /* set reg */
    if ((ret = set_reg_value (reg_add, val)) < 0) {
        printf ("failed to set reg %x value %d\n", reg_add, val);
    }

    //装配发送的报文(收发相同，原封不动)
    if (ret == 0) {
        ndp->send_len = 8;
        memcpy (ndp->send_buf, ndp->recv_buf, ndp->send_len);
    }

    return ret;
}

int ModbusSync::modbus_deal_write_more_reg(ModbusSync::modbus_dev_t *ndp)
{
    int ret = -1;
    unsigned char reg_num;
    unsigned short crc_recv, crc_calc;
    unsigned short reg_add;

    reg_num = (ndp->recv_buf[4] << 8) + ndp->recv_buf[5];
    if((reg_num*2) != ndp->recv_buf[6]){
        return -1;
    }
    crc_recv = (ndp->recv_buf[reg_num*2 + 8] << 8) + ndp->recv_buf[reg_num*2 + 7];   //高位在前
    crc_calc = modbus_crc(ndp->recv_buf,reg_num*2+7);
    if (crc_calc != crc_recv) {
        printf ("crc error! %04x, %04x\n", crc_recv, crc_calc);
        return -1;
    }

    reg_add = (ndp->recv_buf[2] << 8)+ndp->recv_buf[3];
    QByteArray a;
    QList<QByteArray> list;

    struct timeval current_time;
    float ms;
    short val;
    float recv_time, zero_time, tel_delay, sync_time, r;
    switch(reg_add)
    {
    case md_wr_reg_test:
        ret = start_measurement(ndp);
        break;
        //    case md_rd_reg_sync:
        //        val = (ndp->recv_buf[7] * 0x100 + ndp->recv_buf[8]);
        //        emit do_sync_freq(val);         //发送频率值

        //        gettimeofday(&current_time, NULL);

        //        us = current_time.tv_usec / 1000.0;
        //        recv_time = us - int(us) + int(us) % 20;
        //        zero_time = 0.100 * (ndp->recv_buf[9] * 0x1000000 + ndp->recv_buf[10] * 0x10000 + ndp->recv_buf[11] * 0x100  + ndp->recv_buf[12]);
        //        tel_delay = 0.100 * (ndp->recv_buf[13] * 0x1000000 + ndp->recv_buf[14] * 0x10000 + ndp->recv_buf[15] * 0x100  + ndp->recv_buf[16]);
        //        qDebug()<<"recv time:"<< recv_time<<"ms";
        //        qDebug()<<"zero time:"<<zero_time<< "ms" ;
        //        qDebug()<<"tel delay:"<<tel_delay<< "ms" ;
        //        sync_time = 200 + recv_time - tel_delay;
        //        while (sync_time >= 20) {
        //            sync_time -= 20;
        //        }
        //        qDebug()<<"sync time:\t"<< sync_time<< "ms\n";

        //        if(tel_delay < 30){
        //            break;
        //        }

        //        r = qAbs(Common::avrage(sync_time_list) - sync_time);
        //        if(sync_time_list.count() == 10){
        //            if( r < 1 || r > 19){     //判断输入值有效，进行同步
        //                Common::time_addusec(current_time, -tel_delay * 1000);
        //                send_sync(current_time.tv_sec, current_time.tv_usec);
        //            }
        //            else{
        //                qDebug()<<Common::avrage(sync_time_list) - sync_time ;
        //                qDebug()<<"\n";
        //            }
        //        }




        //        sync_time_list.append(sync_time);
        //        while(sync_time_list.count() > 10){
        //            sync_time_list.removeFirst();
        //        }
        //        break;
    case md_rd_reg_sensor:          //GPS信息
        qDebug()<<"md_rd_reg_sensor"<<reg_num*2;
        qDebug()<<"temp:"<<(ndp->recv_buf[7] * 0x100 + ndp->recv_buf[8]) / 100.0 <<"°C";
        qDebug()<<"shi du:"<<(ndp->recv_buf[9] * 0x100 + ndp->recv_buf[10] ) / 100.0 << "%";
        a.clear();
        for (int i = 11; i < reg_num*2 + 9; ++i) {
            a.append(QChar::fromLatin1(ndp->recv_buf[i]));
        }
//        qDebug()<<"ascii:"<<a;
        list = a.split(',');
        qDebug()<<list;
        break;
    case md_rd_reg_test:
    case md_rd_reg_sync:            //同步信号
        gettimeofday(&current_time, NULL);          //得到当次时间
//        qDebug()<<"current_time:"<<current_time.tv_usec;

        ms = last_time.tv_usec / 1000.0;            //上一次时间,转化为ms
        recv_time = ms - int(ms) + int(ms) % 20;    //保留小数点取余

        tel_delay = 0.100 * (ndp->recv_buf[13] * 0x1000000 + ndp->recv_buf[14] * 0x10000 + ndp->recv_buf[15] * 0x100  + ndp->recv_buf[16]); //延迟(ms)

        val = (ndp->recv_buf[7] * 0x100 + ndp->recv_buf[8]) / 100;      //频率(Hz)
//        qDebug()<<"freq:"<<val<<"Hz";

//        qDebug()<<"recv time:"<<recv_time<<"ms";            //上一次接收时间
//        qDebug()<<"tel delay:"<<tel_delay<< "ms" ;          //上一次的延迟

//        qDebug()<<"source:"<<hex<<ndp->recv_buf[17] * 0x100 + ndp->recv_buf[18];      //同步模式

//        if(tel_delay < 30){
//            break;
//        }

        sync_time = 200 + recv_time - tel_delay;
        while (sync_time >= 20) {
            sync_time -= 20;
        }
//        qDebug()<<"sync time:\t"<< sync_time<< "ms\n";      //上一次同步时间

        if(tel_delay > 20 && tel_delay < 45)
            qDebug()<< QString::number(recv_time,'f',1) << "\t" << QString::number(tel_delay,'f',1) << "\t" << QString::number(sync_time,'f',1);

#if 0
        r = qAbs(Compute::phase_error(sync_time, sync_time_list));
        if(sync_time_list.count() == 8){
            if( r < 1 ){     //判断输入值有效，进行同步
                Common::time_addusec(last_time, -tel_delay * 1000);
                send_sync(last_time.tv_sec, last_time.tv_usec);

                qDebug()<<"success, error is:\t"<<r << "\tavrage is:\t" << Common::avrage(sync_time_list) << sync_time_list;
            }
            else{
                qDebug()<<"failed, error is:\t"<<r << "\tavrage is:\t" << Common::avrage(sync_time_list) << sync_time_list;
            }


            qDebug()<<"\n";
        }

        sync_time_list.append(sync_time);
        while(sync_time_list.count() > 8){
            sync_time_list.removeFirst();
        }
#endif

        last_time = current_time;                           //保存本次时间
        break;
    default:
        break;
    }

    if (ret == 0) {
        ndp->send_len = 8;
        memcpy (ndp->send_buf, ndp->recv_buf, 6);
        crc_calc = modbus_crc (ndp->send_buf, 6);
        ndp->send_buf [7] = crc_calc / 0x100;
        ndp->send_buf [6] = crc_calc % 0x100;
    }
    return ret;
}

//读装置数据
int ModbusSync::get_reg_value(unsigned short reg, unsigned short *val)
{
    //    if (reg < md_rd_reg_dev_st || reg >= md_rd_reg_max) {
    if (reg >= md_rd_reg_max) {
        return -1;
    }

    transData();

    switch (reg) {
    case md_rd_reg_dev_st:
    case md_rd_reg_tev_mag:
    case md_rd_reg_tev_cnt:
    case md_rd_reg_tev_severity:
    case md_rd_reg_aa_mag:
    case md_rd_reg_aa_severity:
    case md_rd_reg_tev_zero_sug:
    case md_rd_reg_tev_noise_sug:
    case md_rd_reg_aa_bias_sug:
    case md_rw_reg_tev_gain:
    case md_rw_reg_tev_noise_bias:
    case md_rw_reg_tev_zero_bias:
    case md_rw_reg_aa_gain:
    case md_rw_reg_aa_bias:
        * val = data_stand[reg];
        qDebug()<<"modbus read successed! val = "<<data_stand[reg];
        break;
    default:
        return -1;
    }

    return 0;
}

int ModbusSync::set_reg_value(unsigned short reg, unsigned short val)
{

    if (reg < md_rw_reg_tev_gain || reg >= md_wr_reg_max) {
        return -1;
    }


    sql_para = sqlcfg->get_para();

    switch (reg) {
    case md_rw_reg_tev_gain:
        sql_para->tev1_sql.gain = val / 10.0;
        break;
    case md_rw_reg_tev_noise_bias:
        //      sql_para->tev1_sql.tev_offset1 = val;
        break;
    case md_rw_reg_tev_zero_bias:
        sql_para->tev1_sql.fpga_zero = - val;
        break;
    case md_rw_reg_aa_gain:
        //      val = ((float)val) / 10;
        //      sql_para->aaultra_sql.gain = val / 10.0;
        break;
    case md_rw_reg_aa_bias:
        //      sql_para->aaultra_sql.aa_offset = val;
        break;
    case md_wr_reg_start:
        sql_para->close_time = 0;
        emit closeTimeChanged(0);
        break;
    case md_wr_reg_stop:
        break;
    case md_rd_reg_sync:
        printf("freq = %d \n",val);
        qDebug()<<"                                   begin"<<QTime::currentTime();
        emit do_sync_immediately();
        emit do_sync_freq(val);
        break;
    default:
        return -1;
    }

    //    sqlcfg->sql_save(sql_para);

    return 0;
}

void ModbusSync::transData()
{
    data_stand[md_rd_reg_dev_st] = (VERSION_MAJOR<<12) + (VERSION_MINOR<<8);        //存储版本号

    //从rdb中取数据(易变量)
    yc_data_type temp_data;
    unsigned char a[1],b[1];

    //   yc_get_value(0,TEV1_amplitude,1, &temp_data, b, a);
    data_stand[md_rd_reg_tev_mag] = temp_data.f_val;

    //  yc_get_value(0,TEV1_num,1, &temp_data, b, a);
    data_stand[md_rd_reg_tev_cnt] = temp_data.f_val;

    //   yc_get_value(0,AA1_amplitude,1, &temp_data, b, a);
    data_stand[md_rd_reg_aa_mag] = temp_data.f_val;

    //  yc_get_value(0,TEV1_center_biased_adv,1, &temp_data, b, a);
    data_stand[md_rd_reg_tev_zero_sug] = temp_data.f_val;

    //   yc_get_value(0,TEV1_noise_biased_adv,1, &temp_data, b, a);
    data_stand[md_rd_reg_tev_noise_sug] = temp_data.f_val;

    //   yc_get_value(0,AA1_biased_adv,1, &temp_data, b, a);
    data_stand[md_rd_reg_aa_bias_sug] = temp_data.f_val;

    //逻辑判断
    sql_para = sqlcfg->get_para();
    unsigned short val = data_stand[md_rd_reg_tev_mag];
    if(val < sql_para->tev1_sql.low){
        data_stand[md_rd_reg_tev_severity] = 0;
    }
    else if(val < sql_para->tev1_sql.high){
        data_stand[md_rd_reg_tev_severity] = 1;
    }
    else{
        data_stand[md_rd_reg_tev_severity] = 2;
    }

    val = data_stand[md_rd_reg_aa_mag];
    /*    if(val < sql_para->aaultra_sql.low){
        data_stand[md_rd_reg_aa_severity] = 0;
    }
    else if(val < sql_para->aaultra_sql.high){
        data_stand[md_rd_reg_aa_severity] = 1;
    }
    else{
        data_stand[md_rd_reg_aa_severity] = 2;
    } */

    //从SQL中读取
    data_stand[md_rw_reg_tev_gain] = sql_para->tev1_sql.gain;
    //   data_stand[md_rw_reg_tev_noise_bias] = sql_para->tev1_sql.tev_offset1;
    //   data_stand[md_rw_reg_tev_zero_bias] = sql_para->tev1_sql.tev_offset2;

    //   data_stand[md_rw_reg_aa_gain] = sql_para->aaultra_sql.gain;
    //   data_stand[md_rw_reg_aa_bias] = sql_para->aaultra_sql.aa_offset;

}

int ModbusSync::SocketConnected(int sock)
{
    if(sock<=0)
        return 0;
    struct tcp_info info;
    int len=sizeof(info);
    getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    if((info.tcpi_state==TCP_ESTABLISHED))
    {
        //myprintf("socket connected\n");
        return 1;
    } else {
        //myprintf("socket disconnected\n");
        return 0;
    }
}













