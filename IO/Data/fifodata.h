#ifndef FIFODATA_H
#define FIFODATA_H

#include "zynq.h"
#include <QThread>
#include "data.h"
#include "recwave.h"
#include "filetools.h"


#define GROUP_NUM_FLAG          0x1000          //组号标志（实际是复位值，表示传输数据未开始或已完成状态）
#define GROUP_NUM_MAX           16              //组号最大值
#define BUF_SIZE                (GROUP_NUM_MAX * 0x100)         //波形数据缓冲区大小

#define FREQ_REG                0x0001
#define AD_1                    0x0002
#define AD_2                    0x0003
#define AD_3                    0x0004
#define AD_4                    0x0005
#define GROUP_NUM               0x0006
#define RAM_RESET               0x0007
#define AA_VOL                  0x0008
#define BACKLIGHT_REG           0x0009
#define READ_FPGA               0x000a        //读数据标志位
#define TEV_AUTO_REC            0x000b        //自动录波标志位

//读写寄存器
#define write_axi_reg(reg, val) *(volatile unsigned int *)(reg) = val
#define read_axi_reg(reg) *(volatile unsigned int *)(reg)


//用于从FPGA读取数据
//这是一个多线程函数，开启一个新线程，在不影响主界面操作的同时持续监听数据输入
//数据读取单位是G_PARA
class FifoData : public QThread
{
    Q_OBJECT
public:
    explicit FifoData(G_PARA *g_data);

public slots:
    void startRecWave(int mode , int time);       //启动录波，需要主函数建立连接


signals:
    void waveData(qint32 *wave,int len,int mod);  //录波完成信号，并发送录波数据
    void waveData(VectorList,MODE);

private slots:
    void recWaveComplete(VectorList wave,MODE mode);

private:

    quint32 recvdata(void);
    void sendpara(void);

    volatile quint32 *vbase0, *vbase1;
    G_PARA *tdata;

    int *buf;

    MODE mode;      //工作模式

    RecWave *tevData, *AAData;
    FileTools *filetools;   //文件工具，实现保存文件

    void recvRecData();     //接收录波数据

    void read_fpga();
    QTimer *timer;


protected:
    void run(void);

};

#endif // RECVFIFODATA_H
