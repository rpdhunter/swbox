#ifndef MODBUSSYNC_H
#define MODBUSSYNC_H

#include <QThread>
#include <QDebug>
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"
#include <QProcess>
#include <QSerialPort>
#include <QThread>
#include "tcpsocket.h"
#include "Gui/Common/wifi.h"
#define UART_PORT				"/dev/ttyPS0"

#define BUFF_SIZE				300

#define MODBUS_RECV_TO_CNT		3
#define MODBUS_BC_ADDR			0xff
#define MODBUS_MAX_FRAME_LEN	256

#define MODBUS_MAX_REG_CNT		0x30

#define MODBUS_FC_READ_REG		0x03
#define MODBUS_FC_WRITE_A_REG	0x06
#define MODBUS_FC_WRITE_MORE_REG	0x10

#define GPIO_RS485_RW			47
#define RS485_RD				0
#define RS485_WR				1




//modbus类，用于实现modbus协议通信
class ModbusSync : public QThread
{
    Q_OBJECT
public:
    ModbusSync(QObject *parent = NULL,G_PARA *g_data = NULL);
    ~ModbusSync();

    void set_ipaddr(QString ip);
    void close_connect_wifi();      //关闭wifi

    void set_sync_source(int source);           //设置同步源

signals:
    void closeTimeChanged(int m);

    void send_sync(qint64 s,qint64 u);       //发送同步信号
    void do_sync_immediately(/*qint32 u*/);     //发送需
    void do_sync_freq(short f);

public slots:
    void init_socket(int socket);

protected:
    void run(void);

private:

    enum {
        /* read only */
        md_rd_reg_dev_st = 0x0000,
        md_rd_reg_tev_mag,
        md_rd_reg_tev_cnt,
        md_rd_reg_tev_severity,
        md_rd_reg_aa_mag,
        md_rd_reg_aa_severity,     //5

        /* suggest value */
        md_rd_reg_tev_zero_sug,
        md_rd_reg_tev_noise_sug,
        md_rd_reg_aa_bias_sug,     //8

        /* read and write */
        md_rw_reg_tev_gain,
        md_rw_reg_tev_noise_bias,
        md_rw_reg_tev_zero_bias,
        md_rw_reg_aa_gain,
        md_rw_reg_aa_bias,         //13

        md_rd_reg_max,

        /* write only */
        md_wr_reg_start,
        md_wr_reg_stop,            //16

        md_wr_reg_test = 0x0052,    //启动测试
        md_rd_reg_sync = 0x0060,    //同步信号（送的是频率，但是送达时间也是有意义的）
        md_rd_reg_sensor = 0x0061,  //传感器信号（温度，湿度，GPS）
        md_rd_reg_test = 0x0062,  //测试
        md_wr_reg_max,
    };

    typedef struct modbus_dev_s
    {
        int connect_flag;
        int com_fd;
        int socket_fd;
        int connect_fd;

        unsigned char dev_addr;

        unsigned int recv_len;
        unsigned char recv_buf [BUFF_SIZE];
        unsigned int recv_to_cnt;       //接收超时计数器
        unsigned int recv_to_flag;      //接收超时标志

        unsigned int send_len;
        unsigned char send_buf [BUFF_SIZE];
    } modbus_dev_t;

    unsigned short modbus_crc (unsigned char * buf, unsigned char length);

    int modbus_com_clr_to (modbus_dev_t * ndp);         //超时计数器清零
    int modbus_com_recv_to (modbus_dev_t * ndp);        //超时计数器累加
    int modbus_com_recv (modbus_dev_t * ndp, unsigned char * buf, int len);     //分配收到的报文
    int modbus_send_msg (modbus_dev_t * ndp);           //发送modbus报文

    int modbus_deal_msg (modbus_dev_t * ndp);           //处理接收到的报文
    int modbus_deal_read_reg (modbus_dev_t *ndp);
    int modbus_deal_write_a_reg (modbus_dev_t *ndp);
    int modbus_deal_write_more_reg(modbus_dev_t *ndp);
    int get_reg_value (unsigned short reg, unsigned short * val);
    int set_reg_value (unsigned short reg, unsigned short val);

    int socket_supper_tcp(int fd, unsigned char * buf, int len);
    int socket_supper_udp (int fd, unsigned char * buf, int len);
    int start_measurement(modbus_dev_t *ndp);

    modbus_dev_t pd_dev;
    G_PARA *data;
    SQL_PARA *sql_para;
    TcpSocket *tcpsocket;

    void transData ();   //将data转换为标准数组
    unsigned short * data_stand;

    bool is_connect;
    int re_connect_counter;     //重连计数器

    QString ipaddr;
    QList<float> sync_time_list;

    //    float delay;
    //    float recv_time, zero_time, tel_delay, sync_time;

    struct timeval last_time;

    int recv_times_count;     //接收次数计数

    int SocketConnected(int sock);          //检测socket是否连接(目前不起作用)
};

#endif // MODBUSSYNC_H
