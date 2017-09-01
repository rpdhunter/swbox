#include "fifocontrol.h"
#include <QtDebug>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "IO/SqlCfg/sqlcfg.h"

FifoControl::FifoControl(G_PARA *g_data, QObject *parent) : QObject(parent)
{
    tdata = g_data;     //与外部交互的数据指针
    base_init();        //基地址初始化
    regs_init();        //寄存器初始化
    //声音播放
    playVoice = false;
}

void FifoControl::playVoiceData(VectorList wave)
{
    this->wave = wave;
    this->playVoice = true;
}

void FifoControl::stop_play_voice()
{
    playVoice = false;
    qDebug()<<"voice play stopped";
}

int FifoControl::read_nomal_data()
{
    read_fpga(sp_read_fpga_nomal);
    return recv_data (vbase_nomal, (unsigned int *)&(tdata->recv_para_nomal));
}

int FifoControl::read_prpd1_data()
{
    read_fpga(sp_read_fpga_prpd1);
    return recv_data (vbase_prpd1, (unsigned int *)&(tdata->recv_para_prpd1));
}

int FifoControl::read_prpd2_data()
{
    read_fpga(sp_read_fpga_prpd2);
    return recv_data (vbase_prpd2, (unsigned int *)&(tdata->recv_para_prpd2));
}

int FifoControl::read_hfct1_data()
{
    read_fpga(sp_read_fpga_hfct1);
    return recv_data (vbase_hfct1, (unsigned int *)&(tdata->recv_para_hfct1));
}

int FifoControl::read_hfct2_data()
{
    read_fpga(sp_read_fpga_hfct2);
    return recv_data (vbase_hfct2, (unsigned int *)&(tdata->recv_para_hfct2));
}

int FifoControl::read_rec_data()
{
    read_fpga(sp_read_fpga_rec);
    return recv_data (vbase_rec, (unsigned int *)&(tdata->recv_para_rec));
}

void FifoControl::base_init()
{
    //这是xilinx的函数，【可能】用于打开存储设备
    int fd = open (DEV_VMEM, O_RDWR | O_SYNC);

    if (fd == -1) {
        printf ("Open mem device failed!\n");
        return;
    }

    int res = -1;
    do {
        if ((vbase_nomal = init_vbase (fd, AXI_STREAM_BASE0, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase_send = init_vbase (fd, AXI_STREAM_BASE1, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase_play_voice_1 = init_vbase (fd, AXI_STREAM_BASE2, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase_play_voice_2 = init_vbase (fd, AXI_STREAM_BASE3, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase4 = init_vbase (fd, AXI_STREAM_BASE4, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase_hfct1 = init_vbase (fd, AXI_STREAM_BASE5, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase_hfct2 = init_vbase (fd, AXI_STREAM_BASE6, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase_prpd1 = init_vbase (fd, AXI_STREAM_BASE7, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase_prpd2 = init_vbase (fd, AXI_STREAM_BASE8, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase_rec = init_vbase (fd, AXI_STREAM_BASE9, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        res = 0;
    } while (0);

    if (res < 0) {
        close (fd);
        return;
    }
}

volatile unsigned int *FifoControl::init_vbase(int vmem_fd, unsigned int base, unsigned int size)
{
    volatile unsigned int * vbase;

    /* arm get data from fpga fifo */
    vbase = (unsigned int *)mmap (
                NULL,
                size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                vmem_fd,
                base);

    if (vbase == NULL) {
        printf("Mmap 0x%x failed!\n", base);
        return NULL;
    }

    /* basic init operations */
    write_axi_reg (vbase + TDFR, XLLF_TDFR_RESET_MASK);							//reset tx
    write_axi_reg (vbase + ISR, XLLF_INT_ALL_MASK);								//clear all interrupt

    return vbase;
}

void FifoControl::regs_init()
{
    tdata->set_send_para (sp_ram_reset, 1);
    send_para();
    tdata->set_send_para (sp_ram_reset, 0);
    send_para();
    tdata->set_send_para (sp_ram_reset, 1);
    send_para();

    tdata->set_send_para (sp_freq_reg, sqlcfg->get_para()->freq_val);
    tdata->set_send_para (sp_backlight_reg, sqlcfg->get_para()->backlight);
    tdata->set_send_para (sp_keyboard_backlight, 0);//to be

    tdata->set_send_para (sp_working_mode, 0);//to be
    tdata->set_send_para (sp_filter_mode, 0);//to be

    tdata->set_send_para (sp_tev1_zero, 0x8000 - sqlcfg->get_para()->tev1_sql.fpga_zero);
    tdata->set_send_para (sp_tev1_threshold, sqlcfg->get_para()->tev1_sql.fpga_threshold);
    tdata->set_send_para (sp_tev2_zero, 0x8000 - sqlcfg->get_para()->tev2_sql.fpga_zero);
    tdata->set_send_para (sp_tev2_threshold, sqlcfg->get_para()->tev2_sql.fpga_threshold);
    tdata->set_send_para (sp_hfct1_zero, 0x8000 - sqlcfg->get_para()->tev1_sql.fpga_zero);//to be
    tdata->set_send_para (sp_hfct1_threshold, sqlcfg->get_para()->tev1_sql.fpga_threshold);//to be
    tdata->set_send_para (sp_hfct2_zero, 0x8000 - sqlcfg->get_para()->tev2_sql.fpga_zero);//to be
    tdata->set_send_para (sp_hfct2_threshold, sqlcfg->get_para()->tev2_sql.fpga_threshold);//to be

    tdata->set_send_para (sp_aa_vol, sqlcfg->get_para ()->aaultra_sql.vol);
    tdata->set_send_para (sp_aa_record_play, 0);

    tdata->set_send_para (sp_tev_auto_rec, sqlcfg->get_para()->tev1_sql.auto_rec + 2 * sqlcfg->get_para()->tev2_sql.auto_rec);

    send_para();
}

//发送一组数据至fpga寄存器
//会检查全部数据，如果没有数据更改，不进入检查环节，直接跳过函数体
void FifoControl::send_para()
{
    int i;
    if (!tdata->send_para.data_changed) {
        return;
    }
    for (i = 0; i < sp_num; i++) {
        check_send_param (tdata->send_para.send_params, i, spr_maps [i], vbase_send);
    }
    tdata->send_para.data_changed = false;
}

/***************************************
 * 检测发送某一个寄存器值
 * 1.检测这个值是否应当被发送
 * 2.将值和地址合成一个发送数据
 * 3.打印相关信息
 * *************************************/
void FifoControl::check_send_param(RPARA pp[], int index, unsigned int data_mask, volatile unsigned int *vbase)
{
    unsigned int temp;

    if (pp[index].flag) {
        temp = (data_mask << 16) | pp[index].rval;
        send_data (vbase, &temp, 1);
        if(index != sp_read_fpga_nomal && index != sp_read_fpga_rec && index != sp_group_num
                && index != sp_read_fpga_prpd1 && index != sp_read_fpga_hfct1
                && index != sp_read_fpga_prpd2 && index != sp_read_fpga_hfct2){
            qDebug("WRITE_REG = 0x%08x", temp);
        }
        pp[index].flag = false;
    }
}

//具体发送过程
void FifoControl::send_data(volatile unsigned int *vbase, unsigned int data[], unsigned int data_size)
{
    unsigned int i;
    while (sizeof (int) > read_axi_reg (vbase + TDFV));         //看寄存器是否空闲
    for (i = 0; i < data_size; i++) {
        write_axi_reg (vbase + TDFD, data [i]);
    }
    while (sizeof (int) > read_axi_reg (vbase + TDFV));
    write_axi_reg (vbase + TLR, sizeof (int) * data_size);       //设定传输长度

}

//接收数据，从设备将数据读入缓冲区
//返回数据长度
unsigned int FifoControl::recv_data(volatile unsigned int *vbase, unsigned int *buff)
{
    unsigned int len = 0, i;

    //must read the reg
    if ((len = read_axi_reg (vbase + RDFO)) == 0) {
        return 0;
    }

    len = read_axi_reg (vbase + RLR);
    len >>= 2;

    //    qDebug()<<"len = "<<len;

    if (len > 0) {
        for (i = 0; i < len; i++) {
            buff [i] = read_axi_reg (vbase + RDFD);
        }
    }
    return len;
}

void FifoControl::read_fpga(send_params sp_num)
{
    tdata->set_send_para(sp_num, 1);
    send_para();
//    check_send_param (tdata->send_para.send_params,sp_num, spr_maps [sp_num], vbase_send);
    tdata->set_send_para(sp_num, 0);
    send_para();
}

//以下为声音控制函数
void FifoControl::playVoiceData()
{
    if(playVoice){
        int len, i, j;
        unsigned int buff [0x500], temp;
        //    qDebug()<<"IO thread ID: "<<this->currentThreadId();

        do {
            len = recv_data (vbase_play_voice_2, buff);
        } while (len != 0);

        //送开始播放标志
        temp = (AA_RECORD_PLAY << 16) | 1;
        send_data (vbase_send, &temp, 1);

        //先发32组
        for (i = 0; i < 32; ++i) {
            sendAPackage (wave.mid (i /* 256*/ << 8, 256));
        }

        j = 2;
        while (((j /** 16 * 256*/ << 12) + 16 * 256) < wave.length ()) {
            do {
                len = recv_data (vbase_play_voice_2, buff);
            }
            while (len == 0);

            //t = wave.length () / 320000;
            emit playVoiceProgress (j * 100 * 16 * 256 / 320000 , wave.length() * 100 / 320000, true);        //播放进度

            if (!playVoice) {     //接到终止信号
                break;
            }

            if (buff [0] == 1) {
                //发16个包
                for (i = 0; i < 16; ++i) {
                    sendAPackage (wave.mid ((j /** 16 * 256*/ << 12) + (i /** 256*/ << 8), 256));
                }
            }
            j++;
        }

        //送开始播放标志
        temp = (AA_RECORD_PLAY << 16) | 0;
        send_data (vbase_send, &temp, 1);

        emit playVoiceProgress (wave.length () * 100 / 320000, wave.length () * 100 / 320000 , false);        //播放进度

        this->playVoice = false;
    }
}

void FifoControl::sendAPackage(VectorList wave)
{
    int data [256], i;

    if (wave.length () == 256) {
        i = 0;
        foreach (qint32 w, wave) {
            data [i++] = w;
        }
        send_data (vbase_play_voice_1, (unsigned int *)data, 256);
    }
}
