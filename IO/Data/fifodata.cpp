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


    tdata->send_para.recstart.flag = true;
    tdata->send_para.recstart.rval = 0;
    tdata->send_para.groupNum.flag = false;
    tdata->send_para.groupNum.rval = 0;
    tdata->send_para.tev_auto_rec.flag = true;
//    tdata->send_para.tev_auto_rec.rval = true;
    tdata->send_para.tev_auto_rec.rval = sqlcfg->get_para()->tev_auto_rec;

    tevData = new RecWave(g_data, MODE::TEV);
    AAData = new RecWave(g_data, MODE::AA_Ultrasonic);

    connect(tevData,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));
    connect(AAData,SIGNAL(waveData(VectorList,MODE)),this,SLOT(recWaveComplete(VectorList,MODE)));

    /* Start qthread */
    this->start();
}

//开启录波,功能待开发
void FifoData::startRecWave(int mode,int time)
{
    if(mode == 0){
        this->mode = TEV;
    }
    else if(mode == 1){
        this->mode = AA_Ultrasonic;
    }
    else{
        this->mode = Disable;
    }

    qDebug()<<"receive startRecWave signal! ... "<<mode;

    if(mode == TEV){
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
            msleep(200);        //空闲时，休眠时间较长
        }
    }
    exit(0);        //跳出循环，理论上永远不会执行此句？
}

//发送数据至fpga寄存器
void FifoData::sendpara(void)
{

    //送背光
    if (tdata->send_para.blacklight.flag) {
        tdata->send_para.blacklight.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));  //看寄存器是否空闲
        quint32 temp = (BACKLIGHT_REG<<16) | tdata->send_para.blacklight.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("blacklight = 0x%08x", temp);
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
        temp = (AD_1<<16) | tdata->send_para.recstart.rval;
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
        mode = TEV;   //TEV
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
    case TEV:
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















