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

#include "gpio_oper.c"
#include "uart_oper.c"


Modbus::Modbus()
{

}

void Modbus::run()
{
    int len;
    unsigned char recv_buf [300];

    if (init_modbus_dev (&pd_dev) != 0) {
        printf ("failed to init modbus device\n");
//        return -1;
    }

    while (1) {
        len = uart_recv (pd_dev.com_fd, recv_buf, sizeof (recv_buf));
        if (len > 0) {
            show_msg ("recv msg", (char *)recv_buf, len);
            modbus_com_recv (&pd_dev, recv_buf, len);
        }
        else {
            modbus_com_recv_to (&pd_dev);
        }
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
    ndp->baundrate = 9600;
    ndp->recv_len = 0;
    ndp->recv_to_cnt = 0;
    ndp->recv_to_flag = 0;
    ndp->send_len = 0;

    /* export rs485 rw/rd line */
    ndp->rs485_rw_pin = GPIO_RS485_RW;
    if (gpio_open (ndp->rs485_rw_pin, "out") < 0) {
        printf ("failed to export gpio %d\n", GPIO_RS485_RW);
        return -1;
    }
    /* set rd */
    gpio_set (ndp->rs485_rw_pin, RS485_RD);

    /* open uart */
    ndp->com_fd = uart_open (UART_PORT, ndp->baundrate, 0, 8, 1, 'N'); //打开串口，返回文件描述符
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
    int len;
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
            show_msg ("sent msg", (char *)ndp->send_buf, ndp->send_len);
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
        printf ("modbus_deal_msg error\n");
    }

    return ret;
}

int Modbus::modbus_deal_read_reg(Modbus::modbus_dev_t *ndp)
{
    //crc校验
    unsigned short crc_recv, crc_calc;
    unsigned short start_add, reg_count, reg_val;
    int i;

    if (ndp->recv_len != 8) {   //报文长度必须为8
        return -1;
    }

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
    ndp->send_len = 5 + (reg_count << 1);
    ndp->send_buf[0] = ndp->dev_addr;
    ndp->send_buf[1] = MODBUS_FC_READ_REG;
    ndp->send_buf[2] = (reg_count << 1);

    for (i = 0; i < reg_count; i++) {
        reg_val = 0x1122;	// test
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
    val = (ndp->recv_buf[2] << 8)+ndp->recv_buf[3]; //待赋给寄存器的数值

    //赋值操作
    switch (reg_add) {
    case md_wr_reg_start:
        ret = 0;
        break;
    case md_wr_reg_stop:
        ret = 0;
        break;
    default:
        ret= -1;
        break;
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


