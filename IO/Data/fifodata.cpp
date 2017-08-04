#include "fifodata.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "IO/SqlCfg/sqlcfg.h"
#include <QThreadPool>
//#include <time.h>

// send parameters' register map
static unsigned int spr_maps [sp_num] = {
    FREQ_REG,			// sp_freq
    BACKLIGHT_REG,		// sp_backlight
    REC_START_AD1,			// sp_recstart
    REC_START_AD2,			// sp_recstart
    REC_START_AD3,			// sp_recstart
    REC_START_AD4,			// sp_recstart
    GROUP_NUM,			// sp_groupNum
    AA_VOL,				// sp_aa_vol
    READ_FPGA_MODE_0,	// sp_read_fpga_mode0
    TEV_AUTO_REC,		// sp_tev_auto_rec
    AA_RECORD_PLAY,		// sp_aa_record_play
    TEV1_ZERO,			// sp_tev1_zero
    TEV1_THRESHOLD,		// sp_tev1_threshold
    TEV2_ZERO,			// sp_tev2_zero
    TEV2_THRESHOLD,		// sp_tev2_threshold
    WORKING_MODE_AD1,   //sp_working_mode_ad1
    WORKING_MODE_AD2,   //sp_working_mode_ad2
    WORKING_MODE_AD3,   //sp_working_mode_ad3
    WORKING_MODE_AD4,   //sp_working_mode_ad4
    READ_FPGA_MODE_1,   //sp_read_fpga_mode1
    KEYBOARD_BACKLIGHT, //

};

//建立数据连接，完成线程的初始化工作
FifoData::FifoData(G_PARA *g_data)
{
    /* Judge point */
    if (g_data == NULL) {
        return;
    }
    tdata = g_data;     //与外部交互的数据指针

    regInit();      //寄存器初始化

    //开启3个录波通道
    h_channel1 = new RecWave(g_data, MODE::TEV1);   //高速通道1
    h_channel2 = new RecWave(g_data, MODE::TEV2);   //高速通道2
    l_channel1 = new RecWave(g_data, MODE::AA_Ultrasonic);  //低速通道1
    connect(h_channel1,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));
    connect(h_channel2,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));
    connect(l_channel1,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));

    //脉冲通道每隔45ms变化一次
    timer = new QTimer;
    timer->setInterval(45);
    connect(timer,SIGNAL(timeout()),this,SLOT(change_channel()));
    timer->start();
    channel_flag = true;

    //声音播放
    playVoice = false;

    //双通道录波
    rec_double_flag = 0;

    //高频CT模式
    rfct_mode = false;

    isRecording = false;

    /* Start qthread */
    this->start();
}

void FifoData::regInit()
{
    //这是xilinx的函数，【可能】用于打开存储设备
    int fd, res;

    if ((fd = open (DEV_VMEM, O_RDWR | O_SYNC)) == -1) {
        printf ("Open mem device failed!\n");
        return;
    }

    res = -1;
    do {
        if ((vbase0 = init_vbase (fd, AXI_STREAM_BASE0, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase1 = init_vbase (fd, AXI_STREAM_BASE1, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase2 = init_vbase (fd, AXI_STREAM_BASE2, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase3 = init_vbase (fd, AXI_STREAM_BASE3, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase4 = init_vbase (fd, AXI_STREAM_BASE4, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        if ((vbase5 = init_vbase (fd, AXI_STREAM_BASE5, AXI_STREAM_SIZE)) == NULL) {
            break;
        }
        res = 0;
    } while (0);

    if (res < 0) {
        close (fd);
        return;
    }

    init_vbase_2 (vbase1);

    init_send_params (sqlcfg);
}

volatile unsigned int *FifoData::init_vbase(int vmem_fd, unsigned int base, unsigned int size)
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

void FifoData::init_vbase_2 (volatile unsigned int * vbase)
{
    while (sizeof (int) > read_axi_reg (vbase + TDFV));
    write_axi_reg (vbase + TDFD, 0x00070001);
    write_axi_reg (vbase + TLR, sizeof (int));

    msleep (100);

    while (sizeof (int) > read_axi_reg(vbase + TDFV));
    write_axi_reg (vbase + TDFD, 0x00070000);
    write_axi_reg (vbase + TLR, sizeof (int));

    msleep (100);

    while (sizeof (int) > read_axi_reg (vbase + TDFV));
    write_axi_reg (vbase + TDFD, 0x00070001);
    write_axi_reg (vbase + TLR, sizeof (int));

    msleep (100);

    while (sizeof (int) > read_axi_reg (vbase + TDFV));
    write_axi_reg (vbase + TDFD, 0x00200001);
    write_axi_reg (vbase + TLR, sizeof (int));
}

void FifoData::init_send_params (SqlCfg * psc)
{
    tdata->set_send_para (sp_freq, psc->get_para()->freq_val);
    tdata->set_send_para (sp_backlight, psc->get_para()->backlight);
    tdata->set_send_para (sp_recstart_ad1, 0);
    tdata->set_send_para (sp_recstart_ad2, 0);
    tdata->set_send_para (sp_recstart_ad3, 0);
    tdata->set_send_para (sp_recstart_ad4, 0);
    tdata->set_send_para (sp_groupNum, 0);		//tdata->send_para.groupNum.flag = false;?????
    tdata->set_send_para (sp_aa_vol, psc->get_para ()->aaultra_sql.vol);
    tdata->set_send_para (sp_tev_auto_rec, psc->get_para()->tev1_sql.auto_rec + 2 * psc->get_para()->tev2_sql.auto_rec);
    tdata->set_send_para (sp_aa_record_play, 0);
    tdata->set_send_para (sp_tev1_zero, 0x8000 - psc->get_para()->tev1_sql.fpga_zero);
    tdata->set_send_para (sp_tev1_threshold, psc->get_para()->tev1_sql.fpga_threshold);
    tdata->set_send_para (sp_tev2_zero, 0x8000 - psc->get_para()->tev2_sql.fpga_zero);
    tdata->set_send_para (sp_tev2_threshold, psc->get_para()->tev2_sql.fpga_threshold);
    tdata->set_send_para (sp_working_mode_ad1, 0);
    tdata->set_send_para (sp_working_mode_ad2, 0);
    tdata->set_send_para (sp_working_mode_ad3, 0);
    tdata->set_send_para (sp_working_mode_ad4, 0);
}

//开启录波,功能待开发
void FifoData::startRecWave(MODE mode,int time)
{
    switch (mode) {
    case TEV1:
        h_channel1->recStart(mode);
        break;
    case TEV2:
    case RFCT:
    case RFCT_CONTINUOUS:
        h_channel2->recStart(mode);
        break;
    case AA_Ultrasonic:
        l_channel1->recStart(mode,time);
        break;
    default:
        break;
    }

    isRecording = true;
    qDebug()<<"receive startRecWave signal! ... "<<mode;
    send_para ();
}

//录播完成的处理
//使用了一次性执行的多线程处理模式——QRunnable
//经实际测试，线程能自动结束
void FifoData::recWaveComplete(VectorList wave, MODE mode)
{
    if(rec_double_flag == 0){
        emit waveData(wave,mode);
    }
    else if(rec_double_flag == 2){
        rec_double = wave;
        rec_double_flag -- ;
    }
    else if(rec_double_flag == 1){
        rec_double.append(wave);
        rec_double_flag -- ;
        emit waveData(rec_double,TEV_Double);   //发送拼接好的双通道数据(保存操作在界面完成)

    }

    isRecording = false;

}

void FifoData::change_channel()
{
    unsigned int val;

    //每隔一段时间，改变组号，为了改变FPGA发送的脉冲数据通道
    //加判断条件是为了防止录波时造成组号混乱
    if (h_channel1->status == RecWave::Free && h_channel2->status == RecWave::Free && l_channel1->status == RecWave::Free) {
        if (channel_flag) {
            val = 0x0100;
        }
        else {
            val = 0x0000;
        }


        channel_flag = !channel_flag;
        tdata->set_send_para (sp_groupNum, val);
    }
}

void FifoData::stop_play_voice()
{
    playVoice = false;
    qDebug()<<"voice play stopped";
}



void FifoData::run(void)
{
    int ret = 0;

    while (true) {
        //读数据
        if(rfct_mode && !isRecording){  //高频CT模式
            ret = recv_data (vbase5, tdata->recv_para.rfctData);       //接收数据
            send_para ();
            read_fpga_mode1 ();
            if (ret > 250) {
                tdata->recv_para.groupNum ++;
                if(tdata->recv_para.groupNum == 16){
                    tdata->recv_para.groupNum = 0;
                }
                msleep(2);
            }
            else {
                msleep(180);         //没数据，休息18ms
            }
        }
        else{       //普通模式和录波模式
            ret = recv_data (vbase0, (unsigned int *)&(tdata->recv_para));       //接收数据
            //录波
            if (ret) {
    //                        qDebug()<<"recv recComplete = "<<tdata->recv_para.recComplete;      //打印收到的录播完成标志位
                if( tdata->recv_para.recComplete >0 && tdata->recv_para.recComplete <=15){       //录波完成可能值为0-15
                    recvRecData();  //开始接收数据(暂时禁用)
                }
            }

            send_para ();
            read_fpga_mode0 ();
            if(playVoice){          //播放声音
                playVoiceData();
            }

            if(tdata->recv_para.recComplete >0 && tdata->recv_para.recComplete <=16){
                usleep(1);          //上传录波数据时，休眠时间较短
            }
            else{
                msleep(45);        //空闲时，休眠时间较长
            }
        }
    }
    exit(0);        //跳出循环，理论上永远不会执行此句？
}

void FifoData::check_send_param (RPARA pp [], int index, unsigned int data_mask, volatile unsigned int * vbase)
{
    unsigned int temp;

    if (pp [index].flag) {
        temp = (data_mask << 16) | pp [index].rval;
        send_data (vbase, &temp, 1);
        if(index != sp_read_fpga_mode0 && index != sp_read_fpga_mode1 && index != sp_groupNum ){
            //        if(index != sp_groupNum){
            qDebug("WRITE_REG = 0x%08x", temp);
        }
        pp [index].flag = false;
    }
}

//发送数据至fpga寄存器
void FifoData::send_para (void)
{
    int i;

    if (!tdata->send_para.data_changed) {
        return;
    }

    for (i = 0; i < sp_num; i++) {
        check_send_param (tdata->send_para.send_params, i, spr_maps [i], vbase1);
    }

    tdata->send_para.data_changed = false;
}

void FifoData::playVoiceData()
{
    int len, i, j;
    unsigned int buff [0x500], temp;
    //    qDebug()<<"IO thread ID: "<<this->currentThreadId();

    do {
        len = recv_data (vbase3, buff);
    } while (len != 0);

    //送开始播放标志
    temp = (AA_RECORD_PLAY << 16) | 1;
    send_data (vbase1, &temp, 1);

    //先发32组
    for (i = 0; i < 32; ++i) {
        sendAPackage (wave.mid (i /** 256*/ << 8, 256));
    }

    j = 2;
    while (((j /** 16 * 256*/ << 12) + 16 * 256) < wave.length ()) {
        do {
            len = recv_data (vbase3, buff);
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
    send_data (vbase1, &temp, 1);

    emit playVoiceProgress (wave.length () * 100 / 320000, wave.length () * 100 / 320000 , false);        //播放进度

    this->playVoice = false;
}

void FifoData::playVoiceData(VectorList wave)
{
    this->wave = wave;
    this->playVoice = true;
}

void FifoData::switch_rfct_mode(int f)
{
    tdata->set_send_para (sp_working_mode_ad2, f);
    this->rfct_mode = f;
//    qDebug()<<"rfct mode"<<f;
}

void FifoData::sendAPackage (VectorList wave)
{
    int data [256], i;

    if (wave.length () == 256) {
        i = 0;
        foreach (qint32 w, wave) {
            data [i++] = w;
        }
        send_data (vbase2, (unsigned int *)data, 256);
    }
}

void FifoData::send_data (volatile unsigned int *vbase, unsigned int data [], unsigned int data_size)
{
    unsigned int i;

    while (sizeof (int) > read_axi_reg (vbase + TDFV));  //看寄存器是否空闲
    for (i = 0; i < data_size; i++) {
        write_axi_reg (vbase + TDFD, data [i]);
    }
    while (sizeof (int) > read_axi_reg (vbase + TDFV));
    write_axi_reg (vbase + TLR, sizeof (int) * data_size);       //设定传输长度
}

//接收数据，从设备将数据读入缓冲区
//返回数据长度
unsigned int FifoData::recv_data (volatile unsigned int * vbase, unsigned int * buff)
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


void FifoData::recvRecData ()
{
    //由于可能存在自发录播，且可能存在多路同事录播，需要根据FPGA反馈进行模式判断
    //多路同时录波，约定首先传送AD编号较小的通道数据
    //
    //下面算法基于以上生成
    MODE mode;
    int x = tdata->recv_para.recComplete;
    if (x == 3) {
        rec_double_flag = 2;  //同步录波模式
    }

    if (x > 15 || x <= 0) {
        mode = Disable;
    }
    else if (x & 1) {
        mode = TEV1;   //TEV1
    }
    else if (x & 2) {
        mode = TEV2;   //TEV2
    }
    else if (x & 4) {
        mode = AA_Ultrasonic;   //AA超声
    }
    else{           //AD4暂时不用
        mode = Disable;
    }

    switch (mode) {
    case TEV1:
        if(h_channel2->status == RecWave::Working){
//            qDebug()<<"TEV2 Working --> Waiting";
            h_channel2->status = RecWave::Waiting;        //TEV1优先级高于TEV2 Working --> Waiting
        }
        if(h_channel1->status == RecWave::Free){
//            qDebug()<<"TEV1 Free --> Working";
            h_channel1->status = RecWave::Working;
//            qDebug()<<"start new rec!!!!!!!!! mode = TEV1!";
            h_channel1->startWork();
        }
        else{
            h_channel1->work();
        }
        break;

    case TEV2:
        switch (h_channel2->status) {
        case RecWave::Free:
            if(h_channel1->status == RecWave::Free){
                h_channel2->status = RecWave::Working;            //Free --> Working
//                qDebug()<<"TEV2 Free --> Working";
                h_channel2->startWork();
            }
            else{
                h_channel2->status = RecWave::Waiting;            //Free --> Waiting
//                qDebug()<<"TEV2 Free --> Waiting";
            }
            break;
        case RecWave::Working:      //继续上传
            //            qDebug()<<"TEV2 go on working";
            h_channel2->work();
            break;
        case RecWave::Waiting:      //改变状态
            if(h_channel1->status == RecWave::Free){
                h_channel2->status = RecWave::Working;            //Waiting --> Working
//                qDebug()<<"TEV2 Waiting --> Working";
                h_channel2->startWork();
            }
            break;
        default:
            break;
        }
        break;

    case AA_Ultrasonic:
        if(l_channel1->status == RecWave::Free){
            l_channel1->status = RecWave::Working;
            l_channel1->startWork();
        }
        else{
            l_channel1->work();
        }

        break;
    default:
        break;
    }
}


void FifoData::read_fpga_mode0()
{
    tdata->set_send_para (sp_read_fpga_mode0, 1);
    check_send_param (tdata->send_para.send_params,sp_read_fpga_mode0, READ_FPGA_MODE_0, vbase1);
    tdata->set_send_para (sp_read_fpga_mode0, 0);
    check_send_param (tdata->send_para.send_params,sp_read_fpga_mode0, READ_FPGA_MODE_0, vbase1);
}

void FifoData::read_fpga_mode1()
{
    tdata->set_send_para (sp_read_fpga_mode1, 0);
    check_send_param (tdata->send_para.send_params,sp_read_fpga_mode1, READ_FPGA_MODE_1, vbase1);
    tdata->set_send_para (sp_read_fpga_mode1, 1);
    check_send_param (tdata->send_para.send_params,sp_read_fpga_mode1, READ_FPGA_MODE_1, vbase1);
}

















