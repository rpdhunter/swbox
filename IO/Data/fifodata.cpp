#include "fifodata.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>


//建立数据连接，完成线程的初始化工作
FifoData::FifoData(G_PARA *g_data)
{
    /* Judge point */
    if (g_data == NULL) {
        return;
    }
    tdata = g_data;     //与外部交互的数据指针


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

    usleep(500000);

    while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
    write_axi_reg(vbase1 + TDFD, 0x00070000);
    write_axi_reg(vbase1 + TLR, sizeof(int));

    usleep(500000);

    while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
    write_axi_reg(vbase1 + TDFD, 0x00070001);
    write_axi_reg(vbase1 + TLR, sizeof(int));

    buf = (int *)malloc(sizeof(int) * 0x200);


    tdata->send_para.recstart.flag = true;
    tdata->send_para.recstart.rval = 0;
    tdata->send_para.groupNum.flag = false;
    tdata->send_para.groupNum.rval = 10;

    /* Start qthread */
    this->start();
}

//开启录波,功能待开发
void FifoData::startRecWave(int mode)
{
    printf("aaa %d\n",mode);

    this->mode = mode;
    switch (mode) {
    case 0:     //地电波
        if(tdata->send_para.recstart.rval != 1){
            tdata->send_para.recstart.rval = 1;
            tdata->send_para.recstart.flag = true;
        }
        printf("receive startRecWave signal!\n");
        sendpara();
        break;
    case 1:     //AA超声

        break;
    case 2:     //

        break;
    case 3:     //

        break;
    case 4:     //

        break;

    default:
        break;
    }

    //录播函数接口，从FPGA读取一组数据
//    int waDa[200];
//    for(int i=0;i<200;i++){
//        waDa[i]=1;
//    }
//    int len=200;
//    emit waveData(waDa,len,mode);
//    emit waveData();

}

void FifoData::run(void)
{
    int ret = 0;

    while (true) {
        ret = recvdata();       //接收数据

        if (ret) {
#if 0
            qDebug("CNT:%lld", tmp++);
            for (int i = ret-2; i < ret; i++) {
                qDebug("0x%08x", *((unsigned int *)buf + i));
            }
            qDebug("\n");
#endif

            memcpy((void *)&(tdata->recv_para), buf, sizeof(int) * ret);    //数据从缓冲区拷贝到接口的数据接收

//            printf("\nret = %d\n",ret); //打印收到信息长度

//            printf("\ntdata->recv_para.recComplete = %d\n",tdata->recv_para.recComplete);
            if(tdata->recv_para.recComplete == 1){
                if(tdata->send_para.recstart.rval != 2){
                    tdata->send_para.recstart.rval = 2;
                    tdata->send_para.recstart.flag = true;
                }
                recvRecData();  //开始接收数据
            }
//            printf("groupNum=%d\n",tdata->send_para.groupNum.rval);  //打印当前组号
#if 0
            qDebug("CNT:%lld", tmp++);
            for (int i = 0; i < ret; i++) {
                qDebug("0x%08x", *((unsigned int *)(&(tdata->recv_para)) + i));
            }
            qDebug("\n");
#endif
        }
        sendpara();

        usleep(50000);     //强制线程休眠0.2(0.05)秒
    }
    exit(0);        //跳出循环，理论上永远不会执行此句？
}

//此函数功能未明了，可能用于装配返回数据的【发送部分】
void FifoData::sendpara(void)
{
    //送背光
    if (tdata->send_para.blacklight.flag) {
        tdata->send_para.blacklight.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));  //看寄存器是否空闲
        write_axi_reg(vbase1 + TDFD, tdata->send_para.blacklight.rval);
        qDebug("blacklight = 0x%08x", tdata->send_para.blacklight.rval);
        write_axi_reg(vbase1 + TLR, sizeof(int));       //设定传输长度
    }


    //送频率
    if (tdata->send_para.freq.flag) {
        tdata->send_para.freq.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        write_axi_reg(vbase1 + TDFD, tdata->send_para.freq.rval);
        qDebug("gridfreq = 0x%08x", tdata->send_para.freq.rval);
        write_axi_reg(vbase1 + TLR, sizeof(int));
    }

    //送组号
    if(tdata->send_para.groupNum.flag){
        tdata->send_para.groupNum.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (GROUP_NUM<<16) | tdata->send_para.groupNum.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("groupNum = 0x%08x", temp);
        write_axi_reg(vbase1 + TLR, sizeof(int));
    }

    //送录波信号
    if(tdata->send_para.recstart.flag){
        tdata->send_para.recstart.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (REC_START<<16) | tdata->send_para.recstart.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("recstart = 0x%08x", temp);
        write_axi_reg(vbase1 + TLR, sizeof(int));
    }

    //送超声音量
    if(tdata->send_para.aa_vol.flag){
        tdata->send_para.aa_vol.flag = false;
        while (sizeof(int) > read_axi_reg(vbase1 + TDFV));
        quint32 temp = (AA_VOL<<16) | tdata->send_para.aa_vol.rval;
        write_axi_reg(vbase1 + TDFD, temp);
        qDebug("aa_vol = 0x%08x", temp);
        write_axi_reg(vbase1 + TLR, sizeof(int));
    }


}


//接收数据，从设备将数据读入缓冲区
//返回数据长度
quint32 FifoData::recvdata(void)
{
    int len = 0;
    int i;

    len = read_axi_reg(vbase0 + RDFO);
//    printf("\n len=%d\n",len);
    //must read the reg
    if (!len) {
        return len;
    }
//    printf("[0]len = %d\n", len);

    len = read_axi_reg(vbase0 + RLR);
//    printf("[1]len = %d\n", len);
    len >>= 2;

//    printf("[2]len = %d\n", len);
    if (len > 0) {
        for (i = 0; i < len; i++) {
            *(buf + i) = read_axi_reg(vbase0 + RDFD);
        }
    }
    return len;
}

//写寄存器
void FifoData::write_axi_reg(volatile quint32 *reg, quint32 val)
{
    *reg = val;
//    printf("aaaaaaaaaaa=0x%08x\n",val);
}

//读
quint32 FifoData::read_axi_reg(volatile quint32 *reg)
{
    return *reg;
}

void FifoData::recvRecData()
{
    if(tdata->send_para.groupNum.rval <8){


        if(tdata->recv_para.groupNum==tdata->send_para.groupNum.rval){
            memcpy(recWaveData+tdata->send_para.groupNum.rval*256,tdata->recv_para.recData,256);

            printf("receive recWaveData!groupNum=%d\n",tdata->send_para.groupNum.rval);

            tdata->send_para.groupNum.rval++;
            tdata->send_para.groupNum.flag = true;
        }
        else{
            printf("receive recWaveData failed! send groupNum=%d\n",tdata->send_para.groupNum.rval);
            printf("recv groupNum=%d\n",tdata->recv_para.groupNum);
            tdata->send_para.groupNum.flag = true;
        }
        sendpara();
    }
    if(tdata->send_para.groupNum.rval==8){
        tdata->send_para.groupNum.rval=10;
        tdata->send_para.groupNum.flag = true;

        tdata->send_para.recstart.rval=0;
        tdata->send_para.recstart.flag = true;
         //接收组装数据完毕
        printf("receive recWaveData complete!\n");
        emit waveData(recWaveData,0x800,mode);
    }

    if(tdata->send_para.groupNum.rval > 8 && tdata->send_para.recstart.rval == 2){       //从初始状态,开始启动数据碎片读取
        tdata->send_para.groupNum.rval=0;
        tdata->send_para.groupNum.flag = true;
    }
}

