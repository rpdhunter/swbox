#ifndef SYNCCOMPUTE_H
#define SYNCCOMPUTE_H

#include <QObject>
#include "Gui/Common/common.h"
#include "gpsinfo.h"
#include <sys/time.h>    // for gettimeofday()

#define BUFF_SIZE                   300
#define MODBUS_BC_ADDR              0xff
#define MODBUS_FC_READ_REG          0x03
#define MODBUS_FC_WRITE_A_REG       0x06
#define MODBUS_FC_WRITE_MORE_REG	0x10

typedef struct modbus_dev_s
{
    unsigned char dev_addr;

    unsigned int recv_len;
    unsigned char recv_buf [BUFF_SIZE];

    unsigned int send_len;
    unsigned char send_buf [BUFF_SIZE];
} modbus_dev_t;

enum {
    md_rd_reg_sync = 0x0060,    //同步信号（送的是频率，但是送达时间也是有意义的）
    md_rd_reg_sensor = 0x0061,  //传感器信号（温度，湿度，GPS）
    md_rd_reg_test = 0x0062,  //测试
    md_wr_reg_max,
};

//"\x00\x00\x00\x00$GNRMC", "021538.000", "A", "3157.2718", "N", "11846.4420", "E", "0.72", "0.00", "080419", "", "", "A*78\r\n\x14\xC2\xBF"

struct GPS_INFO
{

};

class SyncCompute : public QObject
{
    Q_OBJECT
public:
    explicit SyncCompute(QObject *parent = nullptr);
    int read_modbus_packet(char *buf, int len);        //读取一包modbus报文
    static unsigned short modbus_crc (unsigned char * buf, unsigned char length);

    void show_time(QString str);           //显示时间

signals:
    void send_msg(char *buf, int len);      //向外发送数据
    void get_reply();                       //是否收到应答报文
    void get_sync();                        //收到同步报文
    void send_sync(qint64,qint64);          //发送同步时间（秒，微秒）
    void send_freq(float);                  //发送频率
    void send_gps_info(GPSInfo *);          //发送GPS报文

public slots:

private:
    struct timeval last_time;

    QMap<float, timeval> sync_map;      //<标准化后过零时间，原始过零时间>
//    QList<float> temp_list;             //记录收到的温度

    int modbus_deal_msg (modbus_dev_t * ndp);
    int modbus_deal_write_a_reg(modbus_dev_t *ndp);
    int modbus_deal_write_more_reg(modbus_dev_t *ndp);

    int modbus_deal_sync_msg(modbus_dev_t *ndp);
    int modbus_deal_sensor_msg(modbus_dev_t *ndp);

    float trans_time(timeval recv_time, float delay_time);      //根据接收时间和延迟时间,算出过零点时间,并标准化
    float compute_sync_time(QVector<float> list);                //计算平均同步时间
    float compute_delay_time(QVector<float> list);                //计算平均同步时间
    timeval compute_zero_time();            //计算过零时间
    timeval compute_zero_time(QMap<float, timeval> map);    //计算过零时间(核函数)

    GPSInfo *gps_info;
};

#endif // SYNCCOMPUTE_H
