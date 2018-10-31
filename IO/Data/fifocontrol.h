#ifndef FIFOCONTROL_H
#define FIFOCONTROL_H

#include <QObject>
#include <QMutex>
#include "zynq.h"
#include "data.h"

#define DEV_VMEM				"/dev/mem"
//读写寄存器
#define write_axi_reg(reg, val) *(volatile unsigned int *)(reg) = val
#define read_axi_reg(reg) *(volatile unsigned int *)(reg)


class FifoControl : public QObject
{
    Q_OBJECT
public:
    explicit FifoControl(G_PARA *g_data, QObject *parent = 0);
    void read_fpga(send_params sp_n);     //从FPGA读取数据的前置步骤

    int read_normal_data();
    int read_short1_data();
    int read_short2_data();
    int read_ae1_data();
    int read_ae2_data();
    int read_rec_data();
    void play_voice_data();   //向FPGA发送声音数据

    void send_para();       //将所有寄存器数据发送至FPGA


public slots:
    void playVoiceData(VectorList wave);        //检查播放声音开关
    void stop_play_voice();
    void send_sync(qint64 s,qint64 u);       //发送同步信号

signals:
    void playVoiceProgress(int cur, int all, bool);        //返回播放声音的实时进度，cur表示当前播放时间(s),all表示总长度(s)，后一个是是否播完，1未播完，0播完

private:
    // send parameters' register map
    unsigned int spr_maps[sp_num] = {
        //常规功能设置
        RAM_RESET,
        FREQ_REG,
        BACKLIGHT_REG,
        KEYBOARD_BACKLIGHT,
        SLEEPING,
        BUZZER,
        BUZZER_FREQ,
        REBOOT,
        FILTER_MODE,
        L1_CHANNEL_MODE,
        L2_CHANNEL_MODE,
        //通道参数
        H1_ZERO,
        H1_THRESHOLD,
        H2_ZERO,
        H2_THRESHOLD,
        VOL_L1,
        AA_RECORD_PLAY,
        VOL_L2,
        KALMAN_L1,
        KALMAN_L2,
        //录波
        REC_ON,
        REC_START_H1,
        REC_START_H2,
        REC_START_L1,
        REC_START_L2,
        GROUP_NUM,
        AUTO_REC,
        //要通道数据信号
        READ_FPGA_NOMAL,
        READ_FPGA_REC,
        READ_FPGA_HFCT1,
        READ_FPGA_HFCT2,
        READ_FPGA_AE1,
        READ_FPGA_AE2,

    };

    QString send_para_to_string(int val);

    //初始化函数
    void base_init();       //基地址初始化
    volatile unsigned int *init_vbase(int vmem_fd, unsigned int base, unsigned int size);
    void regs_init();       //寄存器初始化

    //发送数据

    void check_send_param(RPARA pp [], int index, unsigned int data_mask);
    void send_data(volatile unsigned int * vbase, unsigned int data [], unsigned int data_size);

    //接收数据
    unsigned int recv_data(volatile unsigned int * vbase, unsigned int * buff);    //从FPGA读取数据


    //声音播放
    void send_a_package(VectorList wave);

    G_PARA * tdata;

    //基地址群
    volatile quint32 *vbase_normal_recv, *vbase_normal_send;
    volatile quint32 *vbase_play_voice, *vbase_play_voice_2;
    volatile quint32 *vbase_sync;
    volatile quint32 *vbase_hfct1, *vbase_hfct2;
    volatile quint32 *vbase_prpd1, *vbase_prpd2;
    volatile quint32 *vbase_ae1, *vbase_ae2;
    volatile quint32 *vbase_rec;

    bool play_voice_flag;     //是否播放声音
    VectorList wave;    //保存播放的声音文件

    QMutex mutex;
};

#endif // FIFOCONTROL_H
