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

#include "gpio_oper.c"
#include "uart_oper.c"
#include "IO/Com/rdb/rdb.h"



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

    //    serial = new QSerialPort;
    //    //设置端口
    //    serial->setPort(QSerialPortInfo::availablePorts().first());
    //    //打开串口
    //    bool flag = serial->open(QIODevice::ReadWrite);
    //    //设置波特率
    //    serial->setBaudRate(9600);
    //    //设置数据位数
    //    serial->setDataBits(QSerialPort::Data8);
    //    //设置奇偶校验
    //    serial->setParity(QSerialPort::NoParity);
    //    //设置停止位
    //    serial->setStopBits(QSerialPort::OneStop);
    //    //设置流控制
    //    serial->setFlowControl(QSerialPort::NoFlowControl);

    //    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));

    //    writeData();


    //    printf("[1]modbus start!");
    //    if(flag)
    //        qDebug()<<"serial open success!";
    //    else
    //        qDebug()<<"serial open failed!";

    if (init_modbus_dev (&pd_dev) != 0) {
        printf ("failed to init modbus device\n");
        //        return -1;
    }

    this->start();
}

int Modbus::get_serial_fd()
{
    return _serial_fd;
}

Modbus::~Modbus()
{
    //    serial->clear();
    //    serial->close();
    //    serial->deleteLater();
}

void Modbus::run()
{
    int len;
    unsigned char recv_buf [300];



    while (1) {
        len = uart_recv (pd_dev.com_fd, recv_buf, sizeof (recv_buf));
        if (len > 0) {
            show_msg ((char *)"recv msg", (char *)recv_buf, len);
            modbus_com_recv (&pd_dev, recv_buf, len);
        }
        else {
            modbus_com_recv_to (&pd_dev);
        }
//        msleep(500);
    }

    close_modbus_dev (&pd_dev);

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

int Modbus::init_modbus_dev(Modbus::modbus_dev_t *ndp)
{
    ndp->com_fd = -1;
    ndp->rs485_rw_pin = -1;
    ndp->dev_addr = 1;
    ndp->baundrate = 115200;
//    ndp->baundrate = 9600;
    ndp->recv_len = 0;
    ndp->recv_to_cnt = 0;
    ndp->recv_to_flag = 0;
    ndp->send_len = 0;

    /* export rs485 rw/rd line */
    ndp->rs485_rw_pin = GPIO_RS485_RW;
    if (gpio_open (ndp->rs485_rw_pin, (char *)"out") < 0) {
        printf ("failed to export gpio %d\n", GPIO_RS485_RW);
//        return -1;
    }
    /* set rd */
    gpio_set (ndp->rs485_rw_pin, RS485_RD);

    /* open uart */
    ndp->com_fd = uart_open ((char*)UART_PORT, ndp->baundrate, 0, 8, 1, 'N'); //打开串口，返回文件描述符

    printf("hellow modbus = %d!\n\n\n ",ndp->com_fd );
    _serial_fd = ndp->com_fd;

    if (ndp->com_fd < 0) {
        printf ("failed to open port %s\n", UART_PORT);
        return -1;
    }

    return 0;
}

int Modbus::close_modbus_dev(Modbus::modbus_dev_t *ndp)
{
    uart_close (ndp->com_fd);
    gpio_close (ndp->rs485_rw_pin);

    return 0;
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
        else if (ndp->recv_len >= 8) {
            /* 满足最小报文长度，处理报文 */
            modbus_deal_msg (ndp);
        }
    }

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

int Modbus::modbus_send_msg(Modbus::modbus_dev_t *ndp)
{
    uint len;
    int sleep_cnt;

    if (ndp->send_len > 0) {
        if (ndp->rs485_rw_pin >= 0) {
            gpio_set (ndp->rs485_rw_pin, RS485_WR);	/* set wr */
        }

        len = uart_send (ndp->com_fd, ndp->send_buf, ndp->send_len);

        if (ndp->rs485_rw_pin >= 0) {
            sleep_cnt = (ndp->send_len * 9000 / ndp->baundrate + 2) * 1000;
            usleep (sleep_cnt);	/* sleep for a while, waiting for send completed */
            gpio_set (ndp->rs485_rw_pin, RS485_RD);	/* set rd */
        }

        if (len == ndp->send_len) {
            show_msg ((char *)"sent msg", (char *)ndp->send_buf, ndp->send_len);
            ndp->send_len = 0;
            return 0;
        }
        else {
            ndp->send_len = 0;
            return -1;
        }
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
        ret = modbus_deal_read_reg (ndp);
        break;
    case MODBUS_FC_WRITE_A_REG:
        ret = modbus_deal_write_a_reg (ndp);
        break;
    default:
        ret = -1;
        break;
    }

    /* clear recv buf */
    ndp->recv_len = 0;

    /* send message */
    if (ndp->send_len > 0) {
        modbus_send_msg (ndp);
    }

    if (ret != 0) {
//        printf ("modbus_deal_msg error\n");
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
    crc_recv = (ndp->recv_buf [6] << 8) + ndp->recv_buf [7];   //高位在前
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
    ndp->send_buf[0] = ndp->dev_addr;
    ndp->send_buf[1] = MODBUS_FC_READ_REG;
    ndp->send_buf[2] = (reg_count << 1);

    transData();        //得到设备数据

    for (i = 0; i < reg_count; i++) {
//        reg_val = 0x1122;	// test
        reg_val = data_stand[start_add + i];
//        if (get_reg_value (start_add + i, &reg_val) < 0) {
//            printf ("failed to get reg %x value\n", start_add + i);
//            reg_val = 0xeeee;	/* invalid value */
//        }

        qDebug()<<"val="<<reg_val;
        ndp->send_buf [3 + (i << 1)] = reg_val >> 8;
        ndp->send_buf [4 + (i << 1)] = reg_val & 0xff;
    }

    //装配CRC校验码
    crc_calc = modbus_crc (ndp->send_buf, 2 * reg_count + 3);
    ndp->send_buf [2 * reg_count + 3] = crc_calc / 0x100;
    ndp->send_buf [2 * reg_count + 4] = crc_calc % 0x100;

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
    crc_recv = (ndp->recv_buf[6] << 8)+ndp->recv_buf[7];   //高位在前
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

int Modbus::show_msg(char *prompt, char buf[], int len)
{
    int i;

    if (buf == NULL ||
            len <= 0) {
        return -1;
    }

    if (prompt != NULL) {
        printf ("%s :", prompt);
    }
    else {
        printf ("msg :");
    }

    for (i = 0; i < len; i++) {
        if ((i & 0xf) == 0) {
            printf ("\n");
        }
        printf (" 0x%02x", buf [i]);
    }
    printf ("\n");

    return 0;
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
//        val = ((float)val) / 10;
        sql_para->tev1_sql.gain = val / 10.0;
        qDebug()<<"tev_gain changed! val = "<<val / 10.0;
        break;
    case md_rw_reg_tev_noise_bias:
        sql_para->tev1_sql.tev_offset1 = val;
        qDebug()<<"tev1_noise changed! val = "<<val;
        break;
    case md_rw_reg_tev_zero_bias:
        sql_para->tev1_sql.fpga_zero = - val;
        qDebug()<<"tev1_zero changed! val = "<<-val;
        break;
    case md_rw_reg_aa_gain:
//        val = ((float)val) / 10;
        sql_para->aaultra_sql.gain = val / 10.0;
        qDebug()<<"aa_gain changed! val = "<<val / 10.0;
        break;
    case md_rw_reg_aa_bias:
        sql_para->aaultra_sql.aa_offset = val;
        qDebug()<<"aa_offset changed! val = "<<val;
        break;
    case md_wr_reg_start:
        sql_para->close_time = 0;
        emit closeTimeChanged(0);
        break;
    case md_wr_reg_stop:
        break;
    default:
        return -1;
    }

    sqlcfg->sql_save(sql_para);

    qDebug()<<"modbus write successed!";


    return 0;
}

void Modbus::transData()
{
    data_stand[md_rd_reg_dev_st] = (VERSION_MAJOR<<12) + (VERSION_MINOR<<8);        //存储版本号

    //从rdb中取数据(易变量)
    yc_data_type temp_data;
    unsigned char a[1],b[1];

    yc_get_value(0,TEV1_amplitude,1, &temp_data, b, a);
    data_stand[md_rd_reg_tev_mag] = temp_data.f_val;
    qDebug()<<"tev_ap" << temp_data.f_val;

    yc_get_value(0,TEV1_num,1, &temp_data, b, a);
    data_stand[md_rd_reg_tev_cnt] = temp_data.f_val;

    yc_get_value(0,AA_amplitude,1, &temp_data, b, a);
    data_stand[md_rd_reg_aa_mag] = temp_data.f_val;

    yc_get_value(0,TEV1_center_biased_adv,1, &temp_data, b, a);
    data_stand[md_rd_reg_tev_zero_sug] = temp_data.f_val;

    yc_get_value(0,TEV1_noise_biased_adv,1, &temp_data, b, a);
    data_stand[md_rd_reg_tev_noise_sug] = temp_data.f_val;

    yc_get_value(0,AA_biased_adv,1, &temp_data, b, a);
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
    if(val < sql_para->aaultra_sql.low){
        data_stand[md_rd_reg_aa_severity] = 0;
    }
    else if(val < sql_para->aaultra_sql.high){
        data_stand[md_rd_reg_aa_severity] = 1;
    }
    else{
        data_stand[md_rd_reg_aa_severity] = 2;
    }

    //从SQL中读取
    data_stand[md_rw_reg_tev_gain] = sql_para->tev1_sql.gain;
    data_stand[md_rw_reg_tev_noise_bias] = sql_para->tev1_sql.tev_offset1;
    data_stand[md_rw_reg_tev_zero_bias] = sql_para->tev1_sql.tev_offset2;

    data_stand[md_rw_reg_aa_gain] = sql_para->aaultra_sql.gain;
    data_stand[md_rw_reg_aa_bias] = sql_para->aaultra_sql.aa_offset;

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



