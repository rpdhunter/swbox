#ifndef FIFODATA_H
#define FIFODATA_H

#include "zynq.h"
#include <QThread>
#include <QDebug>
#include <QList>


#define AD_VAL(val, offset)     ((val) >= offset ? (val) - offset : offset - (val))
#define MAX(a, b)       ((a) > (b) ? (a) : (b))

#pragma pack(1)
typedef union HDATA {
    quint32 ad_data;
    struct {
        quint16 ad_min;
        quint16 ad_max;
    } ad;
} HDATA;

typedef union HPULSE_EDGE {
    quint32 hpulse_edge;
    struct {
        quint16 pos;
        quint16 neg;
    } edge;
} HPULSE_EDGE;

/* fifo recieve data */
typedef struct G_RECV_PARA {
    HDATA hdata0;           //未使用
    HDATA hdata1;           //地电波使用
    quint32 ldata0_max;     //未使用
    quint32 ldata0_min;     //未使用
    quint32 ldata1_max;     //AA超声使用
    quint32 ldata1_min;     //AA超声使用
    HPULSE_EDGE pulse0;
    HPULSE_EDGE pulse1;
    quint32 hpulse0_totol;
    quint32 hpulse1_totol;

    quint32 recComplete;    //录播完成(1为完成)
    quint32 groupNum;       //组号(0-7)
    quint32 recData[259];   //录播数据
} G_RECV_PARA;

typedef struct RPARA {
    bool flag;
    unsigned int rval;
} RPARA;



#define BLACKLIGHT_REG          0x0001
#define FREQ_REG                0x0002
#define REC_START               0x0003
#define UPLOAD_START            0x0004
#define RAM_CLEAR               0x0005
#define GROUP_NUM               0x0006
#define AA_VOL                  0x0008


/* fifo send data */
typedef struct G_SEND_PARA {
    RPARA freq;
    RPARA blacklight;
    RPARA recstart;   //0为常态,1为开始录播,2为开始上传
    RPARA groupNum;     //传输录波数据的组号
    RPARA aa_vol;       //AA超声音量调节
} G_SEND_PARA;

/* total data */
typedef struct G_PARA {
    G_RECV_PARA recv_para;
    G_SEND_PARA send_para;
} G_PARA;
#pragma pack()



//用于从FPGA读取数据
//这是一个多线程函数，开启一个新线程，在不影响主界面操作的同时持续监听数据输入
//数据读取单位是G_PARA
class FifoData : public QThread
{
    Q_OBJECT
public:
    explicit FifoData(G_PARA *g_data);

public slots:
    //GUI发出指令-->FifoData开始录波-->录波完成，发送完成信号
    void startRecWave(int mode);       //启动录波，需要主函数建立连接

signals:
    void waveData(quint32 *wave,int len,int mod);  //录波完成信号，并发送录波数据

private:
    quint32 recvdata(void);
    void sendpara(void);

    void write_axi_reg(volatile quint32 *reg, quint32 val);
    quint32 read_axi_reg(volatile quint32 *reg);


    volatile quint32 *vbase0, *vbase1;
    G_PARA *tdata;
    qint32 fd;

    int *buf;

    volatile quint32 *isr;
    volatile quint32 *rdfr;
    volatile quint32 *rdfo;
    volatile quint32 *rlf;
    volatile quint32 *rdfd;

    int mode;    //录波通道

    quint32 recWaveData[0x800]; //录波数据

    void recvRecData();     //接收录波数据


protected:
    void run(void);

};

#endif // RECVFIFODATA_H
