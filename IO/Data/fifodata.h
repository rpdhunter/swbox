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


//FPGA 寄存器定义
#define FREQ_REG				0x0001          //频率（目前只有50,60两个值，以后可能要扩展成浮点数频率）
#define REC_START_AD1			0x0002          //AD1录波控制信号（0为常态，1为录波开始，2为上传开始）
#define REC_START_AD2           0x0003          //AD2录波控制信号（0为常态，1为录波开始，2为上传开始）
#define REC_START_AD3           0x0004          //AD3录波控制信号（0为常态，1为录波开始，2为上传开始）
#define REC_START_AD4           0x0005          //AD4录波控制信号（0为常态，1为录波开始，2为上传开始）
#define GROUP_NUM				0x0006          //组号(用途1：录波时用于数据组标志，范围(0-15)+通道编号(0x000,0x100,0x200,0x300);用途2：TEV脉冲通道号，常态周期变换)
#define RAM_RESET				0x0007          //初始化时使用（bool）
#define AA_VOL					0x0008          //超声音量（0-15变化，7为原始音量）
#define BACKLIGHT_REG			0x0009          //背光（0-7变化）
#define READ_FPGA_MODE_0		0x000a          //工作模式0时，读数据标志位（bool）
#define TEV_AUTO_REC			0x000b          //自动录波标志位（0为关闭所有自动录波，1为1通道自动，2为2通道自动，3为双通道自动，12为1通道触发同步，20为2通道触发同步，28为双通道触发同步）
#define AA_RECORD_PLAY			0x000c          //播放声音标志(之后发送声音数据至FPGA)（bool）
#define TEV1_ZERO				0x000d          //TEV参考零点(通道1)（0x0000-0xffff）
#define TEV1_THRESHOLD			0x000e          //TEV脉冲阈值(通道1)（0x0000-0xffff）
#define TEV2_ZERO				0x000f          //TEV参考零点(通道2)（0x0000-0xffff）
#define TEV2_THRESHOLD			0x0010          //TEV脉冲阈值(通道2)（0x0000-0xffff）
#define WORKING_MODE_AD1        0x0011          //AD1工作模式，1代表电缆，0代表TEV
#define WORKING_MODE_AD2        0x0012          //AD2工作模式，1代表电缆，0代表TEV
#define WORKING_MODE_AD3        0x0013          //AD3工作模式，1代表AE，0代表AA
#define WORKING_MODE_AD4        0x0014          //AD4工作模式，1代表AE，0代表AA
#define READ_FPGA_MODE_1        0x0015          //工作模式1时，读数据标志位（bool）


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
    void switch_rfct_mode(int f);      //切换到高频CT模式


signals:
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
    volatile quint32 *vbase4, *vbase5;
    G_PARA * tdata;

    RecWave *tevData1, *tevData2, *AAData;
    FileTools *filetools;   //文件工具，实现保存文件

    void recvRecData();     //接收录波数据

    void read_fpga_mode0();
    void read_fpga_mode1();

    void regInit();     //FPGA寄存器初始化

    QTimer *timer;      //通道变化
    bool channel_flag;

    bool playVoice;     //是否播放声音
    VectorList wave;    //保存播放的声音文件

    int rec_double_flag;    //双通道录波的标志位（0为常态，2为双通道录波第一波分，1为双通道录波第二波分）
    VectorList rec_double;  //保存双通道录波的数据

    int rfct_mode;     //是否处于rfct模式

protected:
    void run(void);

};

#endif // RECVFIFODATA_H
