#ifndef FIFODATA_H
#define FIFODATA_H

#include <QThread>
#include "zynq.h"
#include "data.h"
#include "IO/Other/filetools.h"
#include "reccontrol.h"
#include "fifocontrol.h"

//#define DEV_VMEM				"/dev/mem"
////读写寄存器
//#define write_axi_reg(reg, val) *(volatile unsigned int *)(reg) = val
//#define read_axi_reg(reg) *(volatile unsigned int *)(reg)
#define DELAY_TIME_LONG     200
#define DELAY_TIME_SHORT    1

//用于从FPGA读取数据
//这是一个多线程函数，开启一个新线程，在不影响主界面操作的同时持续监听数据输入
//数据读取单位是G_PARA
class FifoData : public QThread
{
    Q_OBJECT
public:
    explicit FifoData(G_PARA *g_data);
    void recvRecData();     //接收录波数据

public slots:
    void startRecWave(MODE mode , int time);       //启动录波，需要主函数建立连接


signals:
    void waveData(VectorList,MODE);     //录波完成信号，并发送录波数据
    void playVoiceData(VectorList wave);            //向FPGA发送声音数据
    void stop_play_voice();
    void playVoiceProgress(int cur, int all, bool);        //返回播放声音的实时进度，前2个参数是播放进度，后一个是是否播完，0未播完，1播完

private:
//    // send parameters' register map
//    unsigned int spr_maps[sp_num] = {
//        //常规功能设置
//        RAM_RESET,
//        FREQ_REG,
//        BACKLIGHT_REG,
//        KEYBOARD_BACKLIGHT,
//        WORKING_MODE,
//        FILTER_MODE,
//        //通道参数
//        TEV1_ZERO,
//        TEV1_THRESHOLD,
//        TEV2_ZERO,
//        TEV2_THRESHOLD,
//        HFCT1_ZERO,
//        HFCT1_THRESHOLD,
//        HFCT2_ZERO,
//        HFCT2_THRESHOLD,
//        AA_VOL,
//        AA_RECORD_PLAY,
//        //录波
//        REC_START_TEV1,
//        REC_START_TEV2,
//        REC_START_HFCT1,
//        REC_START_HFCT2,
//        REC_START_AA1,
//        REC_START_AA2,
//        GROUP_NUM,
//        TEV_AUTO_REC,
//        //要通道数据信号
//        READ_FPGA_NOMAL,
//        READ_FPGA_REC,
//        READ_FPGA_PRPD1,
//        READ_FPGA_PRPD2,
//        READ_FPGA_HFCT1,
//        READ_FPGA_HFCT2
//    };
//    //初始化函数
//    void base_init();       //基地址初始化
//    volatile unsigned int *init_vbase(int vmem_fd, unsigned int base, unsigned int size);
//    void regs_init();       //寄存器初始化

//    //发送数据
//    void send_para();       //将所有寄存器数据发送至FPGA
//    void check_send_param(RPARA pp [], int index, unsigned int data_mask, volatile unsigned int *vbase);
//    void send_data(volatile unsigned int * vbase, unsigned int data [], unsigned int data_size);

//    //接收数据
//    unsigned int recv_data(volatile unsigned int * vbase, unsigned int * buff);    //从FPGA读取数据
//    void read_fpga(send_params sp_num);     //从FPGA读取数据的前置步骤

//    //声音播放
//    void playVoiceData();
//    void sendAPackage(VectorList wave);


    G_PARA * tdata;
    RecControl *reccontrol;   //完成数据接口的杂项控制
//    FileTools *filetools;       //文件工具，实现保存文件
    FifoControl *fifocontrol;
//    //基地址群
//    volatile quint32 *vbase_nomal, *vbase_send;
//    volatile quint32 *vbase_play_voice_1, *vbase_play_voice_2;
//    volatile quint32 *vbase4;
//    volatile quint32 *vbase_hfct1, *vbase_hfct2;
//    volatile quint32 *vbase_prpd1, *vbase_prpd2;
//    volatile quint32 *vbase_rec;
//    //声音播放
//    bool playVoice;     //是否播放声音
//    VectorList wave;    //保存播放的声音文件

protected:
    void run(void);

};

#endif // RECVFIFODATA_H
