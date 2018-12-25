#include "modbus.h"

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

#include <QSerialPortInfo>
#include <QSerialPort>

//#include "IO/Com/modbus/gpio_oper.h"
//#include "IO/Com/modbus/uart_oper.c"
#include "IO/Com/rdb/rdb.h"
//#include "IO/Com/rdb/point_table.h"
#include <QTime>
#include <sys/time.h>    // for gettimeofday()
#include "Gui/Common/common.h"

struct sockaddr_in addr;

Modbus::Modbus(QObject *parent, G_PARA *g_data) : QThread(parent)
{
    int i;


    data = g_data;
    sql_para = new SQL_PARA;
    data_stand = new unsigned short[md_wr_reg_max];
    _serial_fd = -1;

    for (i = 0; i < md_wr_reg_max; ++i) {
        data_stand [i] = 0;
    }
    qDebug()<<"modbus start!";

    tcpsocket = new TcpSocket(this);
    connect(tcpsocket,SIGNAL(connect_ok(int)),this,SLOT(init_socket(int)));
//    tcpsocket->start(QThread::TimeCriticalPriority);

    pd_dev.dev_addr = MODBUS_BC_ADDR;
//    this->start(QThread::TimeCriticalPriority);
}

int Modbus::get_serial_fd()
{
    return _serial_fd;
}

Modbus::~Modbus()
{
}

void Modbus::init_socket(int socket)
{
    pd_dev.socket_fd = socket;
}
void Modbus::run()
{
    int len;
    int ret;
    int old_socket = -1;
    unsigned char recv_buf[300];

    fd_set fds;
    int maxfd;
    struct timeval timeout;

    qDebug()<<"modbus is connected!";
    while (1) {

        if(pd_dev.socket_fd > 0){

            if((pd_dev.socket_fd != old_socket) && (old_socket != -1)){
                qDebug()<<"connect is close";
                close(old_socket);
            }
            old_socket = pd_dev.socket_fd;

            FD_ZERO(&fds);
            FD_SET(old_socket,&fds);
            maxfd = old_socket + 1;
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;
        }else{
            sleep(1);
            continue;
        }

        ret = select(maxfd,&fds,NULL,NULL,&timeout);

        if  ((ret > 0) && (FD_ISSET(old_socket, &fds))){
        //    len = recvfrom(pd_dev.com_fd, recv_buf, sizeof(recv_buf),0,(struct sockaddr *)&addr,&addr_len);
            len = recv(old_socket,recv_buf,sizeof(recv_buf),0);
            if(len <= 0){
                close(old_socket);
                qDebug()<<"recv is close";
                pd_dev.socket_fd = old_socket = -1;
            }

            qDebug()<<"recv: \t"<<QTime::currentTime();
            modbus_com_recv (&pd_dev, recv_buf, len);

#define test_wifi 0
#if test_wifi
            if(recv_buf[0] == 0xaa && recv_buf[1] == 0x55){
                qDebug()<<"recv: \t"<<QTime::currentTime();
                recv_buf[0] = 0;
                recv_buf[1] = 0;
            }
#endif

            int len = 8;
            if(recv_buf[3] == 0x61){
                len = recv_buf[5] * 2;
            }
            else{

                send(pd_dev.socket_fd,"\x55\xaa\x55\xaa",4,0);        //回复信号




//                int ms = QTime::currentTime().msec();
//                qDebug()<<"recv: \t"<<ms<<"\t"<<ms % 20<<"\n";

            }
//            for(int i=0;i<len;i++){
//                printf("%02x  ",recv_buf[i]);
//            }
//            printf("\n");

        }

#if test_wifi
        msleep(3000);
        send(pd_dev.socket_fd,"\x55\xaa",2,0);
        qDebug()<<"send: \t"<<QTime::currentTime();
#endif
    }
}


unsigned short Modbus::modbus_crc(unsigned char *buf, unsigned char length)
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

int Modbus::modbus_com_clr_to(Modbus::modbus_dev_t *ndp)
{
    if (ndp->recv_to_cnt) {
        ndp->recv_to_cnt = 0;
    }
    if (ndp->recv_to_flag) {
        ndp->recv_to_flag = 0;
    }

    return 0;
}

int Modbus::modbus_com_recv(Modbus::modbus_dev_t *ndp, unsigned char *buf, int len)
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

int Modbus::modbus_com_recv_to(Modbus::modbus_dev_t *ndp)
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

int Modbus::socket_supper_udp (int fd, unsigned char * buf, int len)
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

int Modbus::socket_supper_tcp(int fd, unsigned char * buf, int len)
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

int Modbus::start_measurement(Modbus::modbus_dev_t *ndp)
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

int Modbus::modbus_send_msg(Modbus::modbus_dev_t *ndp)
{

    if (ndp->send_len > 0) {

       // socket_supper_udp (ndp->com_fd, ndp->send_buf, ndp->send_len);
        socket_supper_tcp(ndp->socket_fd, ndp->send_buf, ndp->send_len);

        ndp->send_len = 0;
    }

    return 0;
}

int Modbus::modbus_deal_msg(Modbus::modbus_dev_t *ndp)
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

int Modbus::modbus_deal_read_reg(Modbus::modbus_dev_t *ndp)
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

int Modbus::modbus_deal_write_a_reg(Modbus::modbus_dev_t *ndp)
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

int Modbus::modbus_deal_write_more_reg(Modbus::modbus_dev_t *ndp)
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
    float us;
    short val;
    float recv_time, zero_time, tel_delay, sync_time, r;
    switch(reg_add)
    {
    case md_wr_reg_test:
        ret = start_measurement(ndp);
        break;
    case md_rd_reg_sync:
        val = (ndp->recv_buf[7] * 0x100 + ndp->recv_buf[8]);
        emit do_sync_freq(val);         //发送频率值

        gettimeofday(&current_time, NULL);

        us = current_time.tv_usec / 1000.0;
        recv_time = us - int(us) + int(us) % 20;
        zero_time = 0.005 * (ndp->recv_buf[9] * 0x1000000 + ndp->recv_buf[10] * 0x10000 + ndp->recv_buf[11] * 0x100  + ndp->recv_buf[12]);
        tel_delay = 0.005 * (ndp->recv_buf[13] * 0x1000000 + ndp->recv_buf[14] * 0x10000 + ndp->recv_buf[15] * 0x100  + ndp->recv_buf[16]);
        qDebug()<<"recv time:"<< recv_time<<"ms";
        qDebug()<<"zero time:"<<zero_time<< "ms" ;
        qDebug()<<"tel delay:"<<tel_delay<< "ms" ;
        sync_time = 200 + recv_time - tel_delay;
        while (sync_time >= 20) {
            sync_time -= 20;
        }
        qDebug()<<"sync time:\t"<< sync_time<< "ms\n";

        if(tel_delay < 30){
            break;
        }

        r = qAbs(Common::avrage(sync_time_list) - sync_time);
        if(sync_time_list.count() == 10){
            if( r < 1 || r > 19){     //判断输入值有效，进行同步
                Common::time_addusec(current_time, -tel_delay * 1000);
                send_sync(current_time.tv_sec, current_time.tv_usec);
            }
            else{
                qDebug()<<Common::avrage(sync_time_list) - sync_time ;
                qDebug()<<"\n";
            }
        }




        sync_time_list.append(sync_time);
        while(sync_time_list.count() > 10){
            sync_time_list.removeFirst();
        }
        break;
    case md_rd_reg_sensor:
//        qDebug()<<"md_rd_reg_sensor"<<reg_num*2;
//        qDebug()<<"temp:"<<(ndp->recv_buf[7] * 0x100 + ndp->recv_buf[8]) / 100.0 <<"°C";
//        qDebug()<<"shi du:"<<(ndp->recv_buf[9] * 0x100 + ndp->recv_buf[10] ) / 100.0 << "%";
//        a.clear();
//        for (int i = 11; i < reg_num*2 + 9; ++i) {
//            a.append(QChar::fromLatin1(ndp->recv_buf[i]));
//        }
//        qDebug()<<"ascii:"<<a;
//        list = a.split(',');
//        qDebug()<<list;
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
int Modbus::get_reg_value(unsigned short reg, unsigned short *val)
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

int Modbus::set_reg_value(unsigned short reg, unsigned short val)
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

void Modbus::transData()
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



void Modbus::tev_modbus_data(int val, int pluse)
{
    data_stand[md_rd_reg_tev_mag] = val;
    data_stand[md_rd_reg_tev_cnt] = pluse;
//    qDebug()<<"tev modbus data changed! \t " << val << "\t"<< pluse;
}

void Modbus::aa_modbus_data(int val)
{
    data_stand[md_rd_reg_aa_mag] = val;
    //    qDebug()<<"aa modbus data changed!  \t" << val;
}

void Modbus::tev_modbus_suggest(int val1, int val2)
{
    data_stand [md_rd_reg_tev_zero_sug] = val1;
    data_stand [md_rd_reg_tev_noise_sug] = val2;

//    qDebug()<<"tev modbus data changed! \t " << val1 << "\t"<< val2;
}

void Modbus::aa_modbus_suggest(int val)
{
    data_stand [md_rd_reg_aa_bias_sug] = val;
//    qDebug()<<"aa modbus data changed!  \t" << val;
}



