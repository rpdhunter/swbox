#include "fifodata.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "IO/SqlCfg/sqlcfg.h"

#include <QThreadPool>


//建立数据连接，完成线程的初始化工作
FifoData::FifoData(G_PARA *g_data)
{
    /* Judge point */
    if (g_data == NULL) {
        return;
    }
    tdata = g_data;     //与外部交互的数据指针
    mode = Disable;

    regInit();      //寄存器初始化


    tevData = new RecWave(g_data, MODE::TEV1);
    AAData = new RecWave(g_data, MODE::AA_Ultrasonic);

    connect(tevData,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));
    connect(AAData,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));

    timer = new QTimer;
    timer->setInterval(45);
    connect(timer,SIGNAL(timeout()),this,SLOT(change_channel()));

    timer->start();
    channel_flag = true;

    playVoice = false;


    /* Start qthread */
    this->start();
}

void FifoData::regInit()
{
    //这是xilinx的函数，【可能】用于打开存储设备
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
        printf("Open mem device failed!\n");
        return;
    }

    /* arm get data from fpga fifo */
    vbase0 = (unsigned int *)mmap(
            NULL,
            AXI_STREAM_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd,
            AXI_STREAM_BASE0);

    if (vbase0 == NULL) {
        printf("Mmap AXI_STREAM_BASE0 failed!\n");
        close(fd);
        return;
    }

    /* arm sent data to fpga fifo */
    vbase1 = (unsigned int *)mmap(
            NULL,
            AXI_STREAM_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd,
            AXI_STREAM_BASE1);
    if (vbase1 == NULL) {
        printf("Mmap AXI_STREAM_BASE1 failed!\n");
        munmap((void *)vbase0, AXI_STREAM_SIZE);
        close(fd);
        return;
    }


    write_axi_reg(vbase1 + TDFR, XLLF_TDFR_RESET_MASK);							//reset tx
    write_axi_reg(vbase1 + ISR, XLLF_INT_ALL_MASK);								//clear all interrupt

    write_axi_reg(vbase0 + RDFR, XLLF_RDFR_RESET_MASK);							//reset rx
    write_axi_reg(vbase0 + ISR, XLLF_INT_ALL_MASK);

    /* arm sent data to fpga fifo */
    vbase2 = (unsigned int *)mmap(
            NULL,
            AXI_STREAM_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd,
            AXI_STREAM_BASE2);
    if (vbase2 == NULL) {
        printf("Mmap AXI_STREAM_BASE1 failed!\n");
        munmap((void *)vbase3, AXI_STREAM_SIZE);
        close(fd);
        return;
    }
    

    /* arm get data from fpga fifo */
    vbase3 = (unsigned int *)mmap(
            NULL,
            AXI_STREAM_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd,
            AXI_STREAM_BASE3);

    if (vbase3 == NULL) {
        printf("Mmap AXI_STREAM_BASE0 failed!\n");
        close(fd);
        return;
    }

    write_axi_reg(vbase2 + TDFR, XLLF_TDFR_RESET_MASK);							//reset tx
    write_axi_reg(vbase2 + ISR, XLLF_INT_ALL_MASK);								//clear all interrupt

    write_axi_reg(vbase3 + RDFR, XLLF_RDFR_RESET_MASK);							//reset rx
    write_axi_reg(vbase3 + ISR, XLLF_INT_ALL_MASK);


    while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
    write_axi_reg(vbase1 + TDFD, 0x00070001);
    write_axi_reg(vbase1 + TLR, sizeof(int));

    msleep(200);

    while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
    write_axi_reg(vbase1 + TDFD, 0x00070000);
    write_axi_reg(vbase1 + TLR, sizeof(int));

    msleep(200);

    while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
    write_axi_reg(vbase1 + TDFD, 0x00070001);
    write_axi_reg(vbase1 + TLR, sizeof(int));

    buf = (int *)malloc(sizeof(int) * 0x500);

    tdata->send_para.backlight.flag = true;
    tdata->send_para.backlight.rval = sqlcfg->get_para()->backlight;
    tdata->send_para.freq.flag = true;
    tdata->send_para.freq.rval = sqlcfg->get_para()->freq_val;
    tdata->send_para.recstart.flag = true;
    tdata->send_para.recstart.rval = 0;
    tdata->send_para.groupNum.flag = false;
    tdata->send_para.groupNum.rval = 0;
    tdata->send_para.tev_auto_rec.flag = true;
    tdata->send_para.tev_auto_rec.rval = sqlcfg->get_para()->tev1_sql.auto_rec + 2 * sqlcfg->get_para()->tev2_sql.auto_rec;
    tdata->send_para.tev1_zero.flag = true;
    tdata->send_para.tev1_zero.rval = ( 0x8000 - sqlcfg->get_para()->tev1_sql.fpga_zero );
    tdata->send_para.tev1_threshold.flag = true;
    tdata->send_para.tev1_threshold.rval = sqlcfg->get_para()->tev1_sql.fpga_threshold;
    tdata->send_para.tev2_zero.flag = true;
    tdata->send_para.tev2_zero.rval = ( 0x8000 - sqlcfg->get_para()->tev2_sql.fpga_zero );
    tdata->send_para.tev2_threshold.flag = true;
    tdata->send_para.tev2_threshold.rval = sqlcfg->get_para()->tev2_sql.fpga_threshold;
}

//开启录波,功能待开发
void FifoData::startRecWave(int mode,int time)
{
    if(mode == 0){
        this->mode = TEV1;
    }
    else if(mode == 1){
        this->mode = TEV2;
    }
    else if(mode == 2){
        this->mode = AA_Ultrasonic;
    }
    else{
        this->mode = Disable;
    }

    qDebug()<<"receive startRecWave signal! ... "<<mode;

    if(mode == TEV1){
        tevData->recStart();
    }
    else if(mode == AA_Ultrasonic){
        AAData->recStart(time);
    }
    else{
        //to be
    }
    sendpara();
}

//录播完成的处理
//使用了一次性执行的多线程处理模式——QRunnable
//经实际测试，线程能自动结束
void FifoData::recWaveComplete(VectorList wave, MODE mode)
{
    emit waveData(wave,mode);
    filetools = new FileTools(wave,mode);      //开一个线程，为了不影响数据接口性能
    QThreadPool::globalInstance()->start(filetools);
}

void FifoData::change_channel()
{
    //每隔一段时间，改变组号，为了改变FPGA发送的脉冲数据通道
    if(tevData->status == RecWave::Free && AAData->status == RecWave::Free){
        if(channel_flag){
            tdata->send_para.groupNum.rval = 0x0100;
        }
        else{
            tdata->send_para.groupNum.rval = 0x0000;
        }


        channel_flag = !channel_flag;
        tdata->send_para.groupNum.flag = true;
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
//    int tmp = 0;

    while (true) {

        ret = recvdata();       //接收数据

        if (ret) {            
#if 0
            for (int i = 0; i < 15; i++) {
                qDebug("0x%08x", *((unsigned int *)buf + i));
            }
            qDebug("\n");
#endif
            memcpy((void *)&(tdata->recv_para), buf, sizeof(int) * ret);    //数据从缓冲区拷贝到接口的数据接收

//            qDebug()<<"ret = "<<ret;                                            //打印收到信息长度
//            qDebug()<<"recv recComplete = "<<tdata->recv_para.recComplete;      //打印收到的录播完成标志位

            if( tdata->recv_para.recComplete >0 && tdata->recv_para.recComplete <=15){       //录波完成可能值为0-15
//                qDebug()<<"send groupNum = "<<tdata->send_para.groupNum.rval;              //打印当前发送组号
                recvRecData();  //开始接收数据(暂时禁用)
            }

        }

        sendpara();
        read_fpga();

        if(playVoice){          //播放声音
            playVoiceData();
        }

        if(tdata->recv_para.recComplete >0 && tdata->recv_para.recComplete <=16){
            if(tdata->recv_para.recComplete == 1){
                usleep(1);          //上传录波数据时，休眠时间较短
            }
            else if(tdata->recv_para.recComplete == 4){
                usleep(1);          //上传录波数据时，休眠时间较短
            }
            else if(tdata->recv_para.recComplete == 16){
                usleep(1);          //上传录波数据时，休眠时间较短
            }

        }
        else{
            msleep(45);        //空闲时，休眠时间较长
        }
    }
    exit(0);        //跳出循环，理论上永远不会执行此句？
}

//发送数据至fpga寄存器
void FifoData::sendpara(void)
{

    //送背光
    if (tdata->send_para.backlight.flag) {
        tdata->send_para.backlight.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));  //看寄存器是否空闲
        quint32 temp = (BACKLIGHT_REG<<16) | tdata->send_para.backlight.rval;
        write_axi_reg(vbase1 + TDFD, temp);
//        qDebug("blacklight = 0x%08x", temp);
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TLR, sizeof(int));       //设定传输长度
    }

    //送频率
    if (tdata->send_para.freq.flag) {
        tdata->send_para.freq.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (FREQ_REG<<16) | tdata->send_para.freq.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("gridfreq = 0x%08x", temp);
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TLR, sizeof(int));
    }

    //送组号
    if(tdata->send_para.groupNum.flag){
        tdata->send_para.groupNum.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (GROUP_NUM<<16) | tdata->send_para.groupNum.rval;
        write_axi_reg(vbase1 + TDFD, temp);
//        qDebug("groupNum = 0x%08x", temp);
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TLR, sizeof(int));
    }

    //送录波信号
    if(tdata->send_para.recstart.flag){
        tdata->send_para.recstart.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp;
        temp = (REC_START<<16) | tdata->send_para.recstart.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("recstart = 0x%08x", temp);
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TLR, sizeof(int));
    }

    //送超声音量
    if(tdata->send_para.aa_vol.flag){
        tdata->send_para.aa_vol.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (AA_VOL<<16) | tdata->send_para.aa_vol.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("aa_vol = 0x%08x", temp);
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TLR, sizeof(int));
    }

    //送读取信号
    if (tdata->send_para.read_fpga.flag) {
        tdata->send_para.read_fpga.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (READ_FPGA<<16) | tdata->send_para.read_fpga.rval;
        write_axi_reg(vbase1 + TDFD, temp);
//        qDebug("read_fpga = 0x%08x", temp);
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TLR, sizeof(int));       //设定传输长度
    }

    //送自动录波标志
    if (tdata->send_para.tev_auto_rec.flag) {
        tdata->send_para.tev_auto_rec.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (TEV_AUTO_REC<<16) | tdata->send_para.tev_auto_rec.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("TEV_AUTO_REC = 0x%08x", temp);
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TLR, sizeof(int));       //设定传输长度
    }

    //送TEV1零标志
    if (tdata->send_para.tev1_zero.flag) {
        tdata->send_para.tev1_zero.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (TEV1_ZERO<<16) | tdata->send_para.tev1_zero.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("TEV1_ZERO = 0x%08x", temp);
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TLR, sizeof(int));       //设定传输长度
    }

    //送TEV1脉冲阈值
    if (tdata->send_para.tev1_threshold.flag) {
        tdata->send_para.tev1_threshold.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (TEV1_THRESHOLD<<16) | tdata->send_para.tev1_threshold.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("TEV1_THRESHOLD = 0x%08x", temp);
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TLR, sizeof(int));       //设定传输长度
    }

    //送TEV2零标志
    if (tdata->send_para.tev2_zero.flag) {
        tdata->send_para.tev2_zero.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (TEV2_ZERO<<16) | tdata->send_para.tev2_zero.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("TEV2_ZERO = 0x%08x", temp);
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TLR, sizeof(int));       //设定传输长度
    }

    //送TEV2脉冲阈值
    if (tdata->send_para.tev2_threshold.flag) {
        tdata->send_para.tev2_threshold.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (TEV2_THRESHOLD<<16) | tdata->send_para.tev2_threshold.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("TEV2_THRESHOLD = 0x%08x", temp);
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TLR, sizeof(int));       //设定传输长度
    }

}

void FifoData::playVoiceData()
{
//    qDebug()<<"IO thread ID: "<<this->currentThreadId();
    int buff [0x500];
    int len;

    do {
        len = recvdata1(buff);
    }
    while (len != 0);
    //送开始播放标志
    while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
    quint32 temp = (AA_RECORD_PLAY<<16) | 1;
    write_axi_reg(vbase1 + TDFD, temp);
    while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
    write_axi_reg(vbase1 + TLR, sizeof(int));       //设定传输长度

    //先发32组
    for (int i = 0; i < 32; ++i) {
        sendAPackage(wave.mid(i*256,256));
    }

    int j=2,t;

    while(j * 16 * 256 + 16 * 256 < wave.length() ){
        do {
            len = recvdata1(buff);

        }
        while (len == 0);

        t = wave.length() / 320000;
        emit playVoiceProgress( 100 * j * 16 * 256 / 320000 , 100 * wave.length() / 320000, true);        //播放进度

        if(!playVoice){     //接到终止信号
            break;
        }

        if(buff[0] == 1){
            //发16个包
            for (int i = 0; i < 16; ++i) {
                sendAPackage(wave.mid(j * 16 * 256 + i * 256, 256));
            }
        }
        j++;
    }

    //送开始播放标志
    while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
    temp = (AA_RECORD_PLAY<<16) | 0;
    write_axi_reg(vbase1 + TDFD, temp);
    while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
    write_axi_reg(vbase1 + TLR, sizeof(int));       //设定传输长度

    emit playVoiceProgress( 100 * wave.length() / 320000, 100 * wave.length() / 320000 , false);        //播放进度

    this->playVoice = false;
}

void FifoData::playVoiceData(VectorList wave)
{
    this->wave = wave;
    this->playVoice = true;
}

void FifoData::sendAPackage(VectorList wave)
{
    if(wave.length() == 256){
        while (sizeof(int) > read_axi_reg(vbase2 + TDFV));
        foreach (qint32 w, wave) {
            write_axi_reg(vbase2 + TDFD, w);
        }
        while (sizeof(int) > read_axi_reg(vbase2 + TDFV));
        write_axi_reg(vbase2 + TLR, 256 * sizeof(int));       //设定传输长度
    }
}


//接收数据，从设备将数据读入缓冲区
//返回数据长度
quint32 FifoData::recvdata(void)
{
    int len = 0;
    int i;

    len = read_axi_reg(vbase0 + RDFO);

    //must read the reg
    if (!len) {
        return len;
    }

    len = read_axi_reg(vbase0 + RLR);
    len >>= 2;

//    qDebug()<<"len = "<<len;

    if (len > 0) {
        for (i = 0; i < len; i++) {
            *(buf + i) = read_axi_reg(vbase0 + RDFD);
        }
    }
    return len;
}

quint32 FifoData::recvdata1(int *buff)
{
    int len = 0;
    int i;

    len = read_axi_reg(vbase3 + RDFO);

    //must read the reg
    if (!len) {
        return len;
    }

    len = read_axi_reg(vbase3 + RLR);
    len >>= 2;

    if (len > 0) {
        for (i = 0; i < len; i++) {
            *(buff + i) = read_axi_reg(vbase3 + RDFD);
        }
    }
    return len;
}


void FifoData::recvRecData()
{
    //由于可能存在自发录播，且可能存在多路同事录播，需要根据FPGA反馈进行模式判断
    //多路同时录波，约定首先传送AD编号较小的通道数据
    //
    //下面算法基于以上生成
    int x = tdata->recv_para.recComplete;
    if(x>15 || x<=0){
        mode = Disable;
    }
    else if( (x&1) == 1){
        mode = TEV1;   //TEV
    }
    else if( (x&2) == 2){
        mode = Disable;   //AD2暂时不用
    }
    else if( (x&4) == 4){
        mode = AA_Ultrasonic;   //AA超声
    }
    else{           //AD4暂时不用
        mode = Disable;
    }

    switch (mode) {
    case TEV1:
        if(tevData->status == RecWave::Free){
            tevData->status = RecWave::Working;
            tevData->startWork();
        }
        else{
            tevData->work();
        }
        break;

    case AA_Ultrasonic:
        if(tevData->status == RecWave::Working){        //TEV通道正在使用，AA先暂停
            AAData->status = RecWave::Pending;
        }
        else if(AAData->status == RecWave::Free){
            AAData->status = RecWave::Working;
            AAData->startWork();
        }
        else if(AAData->status == RecWave::Pending){
            AAData->status = RecWave::Working;
            AAData->work();
        }
        else{
            AAData->work();
        }

        break;
    default:
        break;
    }
}


void FifoData::read_fpga()
{
    tdata->send_para.read_fpga.flag = true;
    tdata->send_para.read_fpga.rval = 1;
    sendpara();
    usleep(1);
    tdata->send_para.read_fpga.flag = true;
    tdata->send_para.read_fpga.rval = 0;
    sendpara();


}

















