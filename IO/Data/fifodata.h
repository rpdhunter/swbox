#ifndef FIFODATA_H
#define FIFODATA_H

#include "zynq.h"
#include <QThread>
#include "data.h"
#include "recwave.h"
#include "filetools.h"
#include "IO/SqlCfg/sqlcfg.h"

#define DEV_VMEM				"/dev/mem"

#define GROUP_NUM_FLAG          0x1000          //组号标志（实际是复位值，表示传输数据未开始或已完成状态）
#define GROUP_NUM_MAX           16              //组号最大值
#define BUF_SIZE                (GROUP_NUM_MAX * 0x100)         //波形数据缓冲区大小

#define FREQ_REG				0x0001          //频率
#define REC_START				0x0002          //录波开始
//#define AD_2                    0x0003
//#define AD_3                    0x0004
//#define TEV_CHANNEL             0x0005          //脉冲数据通道号
#define GROUP_NUM				0x0006          //组号
#define RAM_RESET				0x0007          //初始化时使用
#define AA_VOL					0x0008          //超声音量
#define BACKLIGHT_REG			0x0009          //背光
#define READ_FPGA				0x000a          //读数据标志位
#define TEV_AUTO_REC			0x000b          //自动录波标志位
#define AA_RECORD_PLAY			0x000c          //播放声音标志(之后发送声音数据至FPGA)
#define TEV1_ZERO				0x000d          //TEV参考零点(通道1)
#define TEV1_THRESHOLD			0x000e          //TEV脉冲阈值(通道1)
#define TEV2_ZERO				0x000f          //TEV参考零点(通道2)
#define TEV2_THRESHOLD			0x0010          //TEV脉冲阈值(通道2)


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
    void playVoiceData(VectorList wave);            //向FPGA发送声音数据


signals:
//    void waveData(qint32 *wave,int len,int mod);  //录波完成信号，并发送录波数据
    void waveData(VectorList,MODE);     //录波完成信号，并发送录波数据
    void playVoiceProgress(int cur, int all, bool);        //返回播放声音的实时进度，前2个参数是播放进度，后一个是是否播完，0未播完，1播完

private slots:
    void recWaveComplete(VectorList wave,MODE mode);
    void change_channel();      //切换通道
    void stop_play_voice();

private:
    volatile unsigned int * init_vbase (int vmem_fd, unsigned int base, unsigned int size);
    void init_vbase_2 (volatile unsigned int * vbase);
	void init_send_params (SqlCfg * psc);

private:
    void send_data (volatile unsigned int * vbase, unsigned int data [], unsigned int data_size);
    unsigned int recv_data (volatile unsigned int * vbase, unsigned int * buff);
	
    void check_send_param (RPARA pp [], int index, unsigned int data_mask, volatile unsigned int *vbase);
    void send_para (void);
    void playVoiceData ();

    void sendAPackage(VectorList wave);

    volatile quint32 *vbase0, *vbase1;
    volatile quint32 *vbase2, *vbase3;
    G_PARA * tdata;

    MODE mode;      //工作模式

    RecWave *tevData, *AAData;
    FileTools *filetools;   //文件工具，实现保存文件

    void recvRecData();     //接收录波数据

    void read_fpga();

    void regInit();     //FPGA寄存器初始化
    QTimer *timer;

    bool channel_flag;

    bool playVoice;     //是否播放声音
    VectorList wave;    //保存播放的声音文件


protected:
    void run(void);

};

#endif // RECVFIFODATA_H
