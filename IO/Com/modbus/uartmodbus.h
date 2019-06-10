#ifndef UARTMODBUS_H
#define UARTMODBUS_H

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

#pragma pack (1)
    typedef struct _AE_send{
        unsigned short  mode;
        int  show_val;
        int    sug_noise_offset;
        int  v_50Hz;
        int  v_100Hz;
        int v_effective;
        int v_peak;
        int prpdbuf[40];
    }AE_send;
#pragma pack ()


//modbus类，用于实现modbus协议通信
class UartModbus : public QThread
{
    Q_OBJECT
public:
    UartModbus(QObject *parent = NULL,G_PARA *g_data = NULL);

    int get_serial_fd();

    ~UartModbus();
    int modbus_AE_send (char buf[],unsigned short len);
    int Converse32(int srcData);
    short Converse16(short srcData);

signals:
    void closeTimeChanged(int m);

public slots:
    void tev_modbus_data(int val, int pluse);
    void aa_modbus_data(int val);

    void tev_modbus_suggest(int val1, int val2);
    void aa_modbus_suggest(int val);

protected:
    void run(void);

private:
//    unsigned char recv_buf [BUFF_SIZE];


    enum {
        /* read only */
        md_rd_reg_dev_st = 0x0000,
        md_rd_reg_tev_mag,
        md_rd_reg_tev_cnt,
        md_rd_reg_tev_severity,
        md_rd_reg_aa_mag,
        md_rd_reg_aa_severity,

        /* suggest value */
        md_rd_reg_tev_zero_sug = 0x0030,
        md_rd_reg_tev_noise_sug,
        md_rd_reg_aa_bias_sug,

        /* read and write */
        md_rw_reg_tev_gain = 0x0040,
        md_rw_reg_tev_noise_bias,
        md_rw_reg_tev_zero_bias,
        md_rw_reg_aa_gain,
        md_rw_reg_aa_bias,

        md_rd_reg_max,

        md_rw_reg_AE_data = 0x0078,


        /* write only */
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

#pragma pack (1)
    typedef struct _modbus_protocol{
        char  adr;
        char  fun;
        unsigned short reg;
    }modbus_protocol;
#pragma pack ()


    unsigned short modbus_crc1(unsigned char * buf, unsigned char length);
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
    int get_reg_value1 (unsigned short reg, unsigned short * val);
    int set_reg_value1 (unsigned short reg, unsigned short val);

    modbus_dev_t pd_dev;
    G_PARA *data;
    SQL_PARA *sql_para;

    void transData ();   //将data转换为标准数组
    unsigned short * data_stand;

    unsigned short state, tevAmplitude, tevPluse, aaAmplitude;

//    QSerialPort *serial;
    int _serial_fd;      //保存一个全局的串口fd值
};

#endif // MODBUS_H
