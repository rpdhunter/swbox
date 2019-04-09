#include "modbus.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <QtDebug>

Modbus::Modbus()
{

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

/********************************************
 * 1.判断超时,超时计数 > MODBUS_RECV_TO_CNT,
 *   则认为超时
 * 2.若超时,缓冲区还有未处理的数据,将数据处理完,
 *   并将超时计数器清零
 * *****************************************/
int Modbus::modbus_com_recv_to(modbus_dev_t *ndp)
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

/********************************************
 * 超时计数器,超时标志清零
 * *****************************************/
int Modbus::modbus_com_clr_to(modbus_dev_t *ndp)
{
    if (ndp->recv_to_cnt) {
        ndp->recv_to_cnt = 0;
    }
    if (ndp->recv_to_flag) {
        ndp->recv_to_flag = 0;
    }

    return 0;
}

/********************************************
 * 接收处理modbus报文的入口函数
 * 1.超时标志清零
 * 2.在报文中寻找有效地址作为Modbus报文头,
 *   并将余下报文数据拷贝至设备缓冲区
 * 3.处理设备接收到的报文
 * *****************************************/
int Modbus::modbus_com_recv(modbus_dev_t *ndp, unsigned char *buf, int len)
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

int Modbus::modbus_send_msg(modbus_dev_t *ndp)
{
    if (ndp->send_len > 0) {

        // socket_supper_udp (ndp->com_fd, ndp->send_buf, ndp->send_len);
        socket_supper_tcp(ndp->socket_fd, ndp->send_buf, ndp->send_len);

        ndp->send_len = 0;
    }

    return 0;
}

/********************************************
 * 接收处理modbus报文的处理函数
 * 1.从报文长度,地址判断有效性
 * 2.根据功能码,分配处理接口
 * 3.缓冲区清零,发送回复报文
 * *****************************************/
int Modbus::modbus_deal_msg(modbus_dev_t *ndp)
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
        ret = modbus_deal_read_reg (ndp);     //读一个或多个寄存器
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

int Modbus::modbus_deal_read_reg(modbus_dev_t *ndp)
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
//    ndp->send_buf[0] = pd_dev.dev_addr;                        //设备地址不一样
    ndp->send_buf[1] = MODBUS_FC_READ_REG;
    ndp->send_buf[2] = (reg_count << 1);

//    transData();        //得到设备数据

    for (i = 0; i < reg_count; i++) {

//        reg_val = data_stand[start_add + i];

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

int Modbus::modbus_deal_write_a_reg(modbus_dev_t *ndp)
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

int Modbus::modbus_deal_write_more_reg(modbus_dev_t *ndp)
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
//    QByteArray a;
//    QList<QByteArray> list;

//    struct timeval current_time;
//    float ms;
//    short val;
//    float recv_time, zero_time, tel_delay, sync_time, r;
//    switch(reg_add)
//    {
//    case md_wr_reg_test:
//        ret = start_measurement(ndp);
//        break;
//    case md_rd_reg_sensor:          //GPS信息
//        qDebug()<<"md_rd_reg_sensor"<<reg_num*2;
//        qDebug()<<"temp:"<<(ndp->recv_buf[7] * 0x100 + ndp->recv_buf[8]) / 100.0 <<"°C";
//        qDebug()<<"shi du:"<<(ndp->recv_buf[9] * 0x100 + ndp->recv_buf[10] ) / 100.0 << "%";
//        a.clear();
//        for (int i = 11; i < reg_num*2 + 9; ++i) {
//            a.append(QChar::fromLatin1(ndp->recv_buf[i]));
//        }
////        qDebug()<<"ascii:"<<a;
//        list = a.split(',');
//        qDebug()<<list;
//        break;
//    case md_rd_reg_test:
//    case md_rd_reg_sync:            //同步信号
//        gettimeofday(&current_time, NULL);          //得到当次时间
////        qDebug()<<"current_time:"<<current_time.tv_usec;

//        ms = last_time.tv_usec / 1000.0;            //上一次时间,转化为ms
//        recv_time = ms - int(ms) + int(ms) % 20;    //保留小数点取余

//        tel_delay = 0.100 * (ndp->recv_buf[13] * 0x1000000 + ndp->recv_buf[14] * 0x10000 + ndp->recv_buf[15] * 0x100  + ndp->recv_buf[16]); //延迟(ms)

//        val = (ndp->recv_buf[7] * 0x100 + ndp->recv_buf[8]) / 100;      //频率(Hz)
////        qDebug()<<"freq:"<<val<<"Hz";

////        qDebug()<<"recv time:"<<recv_time<<"ms";            //上一次接收时间
////        qDebug()<<"tel delay:"<<tel_delay<< "ms" ;          //上一次的延迟

////        qDebug()<<"source:"<<hex<<ndp->recv_buf[17] * 0x100 + ndp->recv_buf[18];      //同步模式

////        if(tel_delay < 30){
////            break;
////        }

//        sync_time = 200 + recv_time - tel_delay;
//        while (sync_time >= 20) {
//            sync_time -= 20;
//        }
////        qDebug()<<"sync time:\t"<< sync_time<< "ms\n";      //上一次同步时间

//        if(tel_delay > 20 && tel_delay < 45)
//            qDebug()<< QString::number(recv_time,'f',1) << "\t" << QString::number(tel_delay,'f',1) << "\t" << QString::number(sync_time,'f',1);

////        r = qAbs(Compute::phase_error(sync_time, sync_time_list));
////        if(sync_time_list.count() == 8){
////            if( r < 1 ){     //判断输入值有效，进行同步
////                Common::time_addusec(last_time, -tel_delay * 1000);
////                send_sync(last_time.tv_sec, last_time.tv_usec);

////                qDebug()<<"success, error is:\t"<<r << "\tavrage is:\t" << Common::avrage(sync_time_list) << sync_time_list;
////            }
////            else{
////                qDebug()<<"failed, error is:\t"<<r << "\tavrage is:\t" << Common::avrage(sync_time_list) << sync_time_list;
////            }


////            qDebug()<<"\n";
////        }




////        sync_time_list.append(sync_time);
////        while(sync_time_list.count() > 8){
////            sync_time_list.removeFirst();
////        }

//        last_time = current_time;                           //保存本次时间
//        break;
//    default:
//        break;
//    }

//    if (ret == 0) {
//        ndp->send_len = 8;
//        memcpy (ndp->send_buf, ndp->recv_buf, 6);
//        crc_calc = modbus_crc (ndp->send_buf, 6);
//        ndp->send_buf [7] = crc_calc / 0x100;
//        ndp->send_buf [6] = crc_calc % 0x100;
//    }
    return ret;
}

int Modbus::get_reg_value(unsigned short reg, unsigned short *val)
{
    //    if (reg < md_rd_reg_dev_st || reg >= md_rd_reg_max) {
    if (reg >= md_rd_reg_max) {
        return -1;
    }

//    transData();

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
//        * val = data_stand[reg];
//        qDebug()<<"modbus read successed! val = "<<data_stand[reg];
        break;
    default:
        return -1;
    }

    return 0;
}

int Modbus::set_reg_value(unsigned short reg, unsigned short val)
{
//    if (reg < md_rw_reg_tev_gain || reg >= md_wr_reg_max) {
//        return -1;
//    }


//    sql_para = sqlcfg->get_para();

//    switch (reg) {
//    case md_rw_reg_tev_gain:
//        sql_para->tev1_sql.gain = val / 10.0;
//        break;
//    case md_rw_reg_tev_noise_bias:
//        //      sql_para->tev1_sql.tev_offset1 = val;
//        break;
//    case md_rw_reg_tev_zero_bias:
//        sql_para->tev1_sql.fpga_zero = - val;
//        break;
//    case md_rw_reg_aa_gain:
//        //      val = ((float)val) / 10;
//        //      sql_para->aaultra_sql.gain = val / 10.0;
//        break;
//    case md_rw_reg_aa_bias:
//        //      sql_para->aaultra_sql.aa_offset = val;
//        break;
//    case md_wr_reg_start:
//        sql_para->close_time = 0;
//        emit closeTimeChanged(0);
//        break;
//    case md_wr_reg_stop:
//        break;
//    case md_rd_reg_sync:
//        printf("freq = %d \n",val);
//        qDebug()<<"                                   begin"<<QTime::currentTime();
//        emit do_sync_immediately();
//        emit do_sync_freq(val);
//        break;
//    default:
//        return -1;
//    }

//    //    sqlcfg->sql_save(sql_para);

    return 0;
}

int Modbus::socket_supper_tcp(int fd, unsigned char *buf, int len)
{
    socklen_t sock_len;
    int write_len, offset;

    sock_len = sizeof(struct sockaddr_in);

    for (offset = 0; len > 0; len -= write_len) {
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

int Modbus::socket_supper_udp(int fd, unsigned char *buf, int len)
{
//    socklen_t sock_len;
//    int write_len, offset;

//    sock_len = sizeof(struct sockaddr_in);

//    for (offset = 0; len > 0; len -= write_len) {
//        write_len = sendto(fd, buf + offset, len, 0, (struct sockaddr *)&addr, sock_len);
//        if (write_len < 0) {
//            return -1;
//        }
//        if (write_len < len) {
//            offset += write_len;
//        }
//    }
//    return 0;
}
