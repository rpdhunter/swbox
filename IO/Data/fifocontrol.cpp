#include "fifocontrol.h"
#include <QtDebug>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "IO/SqlCfg/sqlcfg.h"
#include "Gui/Common/common.h"

FifoControl::FifoControl(G_PARA *g_data, QObject *parent) : QObject(parent)
{
    tdata = g_data;     //与外部交互的数据指针
    base_init();        //基地址初始化
    regs_init();        //寄存器初始化
    //声音播放
    play_voice_flag = false;
}

void FifoControl::playVoiceData(VectorList wave)
{
    qDebug()<<"play wave:"<<wave.count();
    this->wave = wave;
    this->play_voice_flag = true;
}

void FifoControl::stop_play_voice()
{
    play_voice_flag = false;
    qDebug()<<"voice play stopped";
}

QString FifoControl::send_para_to_string(int val)
{
    QString tmp;
    switch (val) {
    case sp_ram_reset:
        tmp = "ram_reset";
        break;
    case sp_freq_reg:
        tmp = "freq_reg";
        break;
    case sp_backlight_reg:
        tmp = "backlight_reg";
        break;
    case sp_keyboard_backlight:
        tmp = "keyboard_backlight";
        break;
    case sp_buzzer:
        tmp = "buzzer";
        break;
    case sp_buzzer_freq:
        tmp = "buzzer_freq";
        break;
    case sp_sleeping:
        tmp = "sleeping";
        break;
    case sp_filter_mode:
        tmp = "filter_mode";
        break;
    case sp_l1_channnel_mode:
        tmp = "l1_channnel_mode";
        break;
    case sp_l2_channnel_mode:
        tmp = "l2_channnel_mode";
        break;
    case sp_h1_zero:
        tmp = "h1_zero";
        break;
    case sp_h1_threshold:
        tmp = "h1_threshold";
        break;
    case sp_h2_zero:
        tmp = "h2_zero";
        break;
    case sp_h2_threshold:
        tmp = "h2_threshold";
        break;
    case sp_vol_l1:
        tmp = "vol_l1";
        break;
    case sp_aa_record_play:
        tmp = "aa_record_play";
        break;
    case sp_vol_l2:
        tmp = "vol_l2";
        break;
    case sp_kalman_l1:
        tmp = "kalman_l1";
        break;
    case sp_kalman_l2:
        tmp = "kalman_l2";
        break;
//    case sp_l1_threshold:
//        tmp = "l1_threshold";
//        break;
//    case sp_l2_threshold:
//        tmp = "l2_threshold";
//        break;
    case sp_rec_start_h1:
        tmp = "rec_start_h1";
        break;
    case sp_rec_start_h2:
        tmp = "rec_start_h2";
        break;
    case sp_rec_start_l1:
        tmp = "rec_start_l1";
        break;
    case sp_rec_start_l2:
        tmp = "rec_start_l2";
        break;
    case sp_group_num:
        tmp = "group_num";
        break;
    case sp_auto_rec:
        tmp = "auto_rec";
        break;
    case sp_rec_on:
        tmp = "rec_on";
        break;
    case sp_read_fpga_normal:
        tmp = "read_fpga_normal";
        break;
    case sp_read_fpga_rec:
        tmp = "read_fpga_rec";
        break;
//    case sp_read_fpga_prpd1:
//        tmp = "read_fpga_prpd1";
//        break;
//    case sp_read_fpga_prpd2:
//        tmp = "read_fpga_prpd2";
        break;
    case sp_read_fpga_hfct1:
        tmp = "read_fpga_hfct1";
        break;
    case sp_read_fpga_hfct2:
        tmp = "read_fpga_hfct2";
        break;
    case sp_read_fpga_ae1:
        tmp = "read_fpga_ae1";
        break;
    case sp_read_fpga_ae2:
        tmp = "read_fpga_ae2";
        break;
    default:
        break;
    }
    return tmp.toUpper();
}

int FifoControl::read_normal_data()
{
//    read_fpga(sp_read_fpga_nomal);
    return recv_data (vbase_normal, (unsigned int *)&(tdata->recv_para_normal));
}

int FifoControl::read_prpd1_data()
{
//    return recv_data (vbase_prpd1, (unsigned int *)&(tdata->recv_para_prpd1));
}

int FifoControl::read_prpd2_data()
{
//    return recv_data (vbase_prpd2, (unsigned int *)&(tdata->recv_para_prpd2));
}

int FifoControl::read_short1_data()
{
//    read_fpga(sp_read_fpga_hfct1);
    return recv_data (vbase_hfct1, (unsigned int *)&(tdata->recv_para_short1));
}

int FifoControl::read_short2_data()
{
//    read_fpga(sp_read_fpga_hfct2);
    return recv_data (vbase_hfct2, (unsigned int *)&(tdata->recv_para_short2));
}

int FifoControl::read_ae1_data()
{
    return recv_data (vbase_ae1, (unsigned int *)&(tdata->recv_para_envelope1));
}

int FifoControl::read_ae2_data()
{
    return recv_data (vbase_ae2, (unsigned int *)&(tdata->recv_para_envelope2));
}

int FifoControl::read_rec_data()
{
//    read_fpga(sp_read_fpga_rec);
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
        if ((vbase_normal = init_vbase (fd, AXI_STREAM_BASE0, AXI_STREAM_SIZE)) == NULL) {
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
        if ((vbase_sync = init_vbase (fd, AXI_STREAM_BASE4, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase_hfct1 = init_vbase (fd, AXI_STREAM_BASE5, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase_hfct2 = init_vbase (fd, AXI_STREAM_BASE6, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
//        if ((vbase_prpd1 = init_vbase (fd, AXI_STREAM_BASE7, AXI_STREAM_SIZE)) == NULL) {
//            break;
//        }
//        if ((vbase_prpd2 = init_vbase (fd, AXI_STREAM_BASE8, AXI_STREAM_SIZE)) == NULL) {
//            break;
//        }
        if ((vbase_ae1 = init_vbase (fd, AXI_STREAM_BASE10, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase_ae2 = init_vbase (fd, AXI_STREAM_BASE11, AXI_STREAM_SIZE)) == NULL) {
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
//    tdata->set_send_para (sp_backlight_reg, sqlcfg->get_para()->backlight);
//    tdata->set_send_para (sp_keyboard_backlight, sqlcfg->get_para()->key_backlight);

//    tdata->set_send_para (sp_filter_mode, 0);//to be
    tdata->set_send_para (sp_buzzer_freq, 100000000/800/2/16);
//    tdata->set_send_para (sp_aa_record_play, 2);        //耳机送2通道

    Common::write_fpga_offset_debug(tdata);

//    tdata->set_send_para (sp_kalman_kg, 4);

    tdata->set_send_para (sp_auto_rec, 0);
    tdata->set_send_para (sp_rec_on, 1);
    tdata->set_send_para (sp_sleeping,1);
    tdata->set_send_para (sp_group_num,0);

    tdata->set_send_para (sp_rec_start_h1, 3);        //初始化使用
    tdata->set_send_para (sp_rec_start_h2, 3);

//    tdata->set_send_para (sp_fpga_sleep, 0);//xwt

    tdata->set_send_para (sp_kalman_l1, 1);
    tdata->set_send_para (sp_kalman_l2, 1);


//    tdata->set_send_para (sp_temp_test, 10);        //（0-63，代表0-63/40秒）


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
        check_send_param (tdata->send_para.send_params, i, spr_maps [i]);
    }
    tdata->send_para.data_changed = false;
}

void FifoControl::send_sync(uint offset)
{
    send_data (vbase_sync, &offset, 1);
    qDebug()<<"sync completed!!! \t sync offset = "<< offset;
}

/***************************************
 * 检测发送某一个寄存器值
 * 1.检测这个值是否应当被发送
 * 2.将值和地址合成一个发送数据
 * 3.打印相关信息
 * *************************************/
void FifoControl::check_send_param(RPARA pp[], int index, unsigned int data_mask)
{
    unsigned int temp;

    if (pp[index].flag) {
        temp = (data_mask << 16) | pp[index].rval;
        send_data (vbase_send, &temp, 1);
        if(index != sp_read_fpga_normal && index != sp_read_fpga_rec && index != sp_group_num
                && index != sp_rec_start_h1 && index != sp_rec_start_h2
                && index != sp_read_fpga_hfct1 && index != sp_read_fpga_ae1
                && index != sp_read_fpga_hfct2 && index != sp_read_fpga_ae2
                ){
//            qDebug("WRITE_REG = 0x%08x", temp);
            qDebug().noquote() << QString("%1 = 0x%2").arg(send_para_to_string(index)).arg(temp,8,16, QLatin1Char('0'));
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

//        qDebug()<<"len = "<<len;

    if (len > 0) {
        for (i = 0; i < len; i++) {
            buff [i] = read_axi_reg (vbase + RDFD);
        }
    }
    return len;
}

void FifoControl::read_fpga(send_params sp_n)
{
    tdata->set_send_para(sp_n, 1);
//    send_para();
    check_send_param (tdata->send_para.send_params,sp_n, spr_maps [sp_n]);
    tdata->set_send_para(sp_n, 0);
//    send_para();
    check_send_param (tdata->send_para.send_params,sp_n, spr_maps [sp_n]);
}

//以下为声音控制函数
void FifoControl::play_voice_data()
{
    if(play_voice_flag){
        int len, i, j;
        unsigned int buff [0x500], temp;

//        do {
//            len = recv_data (vbase_play_voice_2, buff);
//        } while (len != 0);

        //送开始播放标志
        recv_data (vbase_play_voice_2, buff);
        temp = (AA_RECORD_PLAY << 16) | 1;
        send_data (vbase_send, &temp, 1);

        //先发32组
        for (i = 0; i < 32; ++i) {
            send_a_package (wave.mid (i /* 256*/ << 8, 256));
        }

        j = 2;
        while (((j /** 16 * 256*/ << 12) + 16 * 256) < wave.length ()) {
//            do {
//                len = recv_data (vbase_play_voice_2, buff);
//            }
//            while (len == 0);
            usleep(9500);
            recv_data (vbase_play_voice_2, buff);

            emit playVoiceProgress (j * 100 * 16 * 256 / 400000 , wave.length() * 100 / 400000, true);        //播放进度

            if (!play_voice_flag) {     //接到终止信号
                break;
            }

            if (buff [0] == 1) {
                //发16个包
                for (i = 0; i < 16; ++i) {
                    send_a_package (wave.mid ((j /** 16 * 256*/ << 12) + (i /** 256*/ << 8), 256));
                }
            }
            j++;
        }

        //送开始播放标志(结束播放)
        temp = (AA_RECORD_PLAY << 16) | 0;
        send_data (vbase_send, &temp, 1);

        emit playVoiceProgress (wave.length () * 100 / 400000, wave.length () * 100 / 400000 , false);        //播放进度

        this->play_voice_flag = false;
    }
}

void FifoControl::send_a_package(VectorList wave)
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
