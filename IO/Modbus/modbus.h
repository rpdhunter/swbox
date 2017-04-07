#ifndef MODBUS_H
#define MODBUS_H

#include <QThread>
#include <QDebug>
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <QSerialPort>


#define UART_PORT				"/dev/ttyPS0"

#define BUFF_SIZE				300

#define MODBUS_RECV_TO_CNT		3
#define MODBUS_BC_ADDR			0xff
#define MODBUS_MAX_FRAME_LEN	256

#define MODBUS_MAX_REG_CNT		0x30

#define MODBUS_FC_READ_REG		0x03
#define MODBUS_FC_WRITE_A_REG	0x06

#define GPIO_RS485_RW			47
#define RS485_RD				0
#define RS485_WR				1




//modbus类，用于实现modbus协议通信
class Modbus : public QThread
{
    Q_OBJECT
public:
    Modbus(QObject *parent = NULL,G_PARA *g_data = NULL);

    ~Modbus();

signals:


public slots:
    void readData();
    void writeData();


protected:
    void run(void);

private:
//    unsigned char recv_buf [BUFF_SIZE];


    enum {
        md_rd_reg_dev_st = 0x0000,
        md_rd_reg_tev_mag,
        md_rd_reg_tev_cnt,
        md_rd_reg_aa_mag,

        md_rd_reg_max,

        md_wr_reg_start = 0x0100,
        md_wr_reg_stop,

        md_wr_reg_max,
    };

    typedef struct modbus_dev_s
    {
        int com_fd;
        int rs485_rw_pin;
        int baundrate;

        unsigned char dev_addr;

        unsigned int recv_len;
        unsigned char recv_buf [BUFF_SIZE];
        unsigned int recv_to_cnt;
        unsigned int recv_to_flag;

        unsigned int send_len;
        unsigned char send_buf [BUFF_SIZE];
    } modbus_dev_t;

    unsigned short modbus_crc (unsigned char * buf, unsigned char length);
    int init_modbus_dev (modbus_dev_t * ndp);
    int close_modbus_dev (modbus_dev_t * ndp);
    int modbus_com_clr_to (modbus_dev_t * ndp);
    int modbus_com_recv (modbus_dev_t * ndp, unsigned char * buf, int len);
    int modbus_com_recv_to (modbus_dev_t * ndp);
    int modbus_send_msg (modbus_dev_t * ndp);

    int modbus_deal_msg (modbus_dev_t * ndp);
    int modbus_deal_read_reg (modbus_dev_t *ndp);
    int modbus_deal_write_a_reg (modbus_dev_t *ndp);
    int show_msg(char *prompt, char buf[], int len);

    modbus_dev_t pd_dev;
    G_PARA *data;
    SQL_PARA *sql_para;

    void transData();   //将data转换为标准数组
    unsigned short *data_stand;

    unsigned short state, tevAmplitude, tevPluse, aaAmplitude;

    QSerialPort *serial;




};

#endif // MODBUS_H
