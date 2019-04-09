#include "synccompute.h"
#include <sys/time.h>    // for gettimeofday()

/*********************************************************
 * modbus报文处理类,主要处理3类报文
 * 1.GPS报文
 * 2.同步报文
 * 3.保活应答报文
 * ******************************************************/
SyncCompute::SyncCompute(QObject *parent) : QObject(parent)
{
}

/*********************************************************
 * modbus报文处理函数一
 * 1.完成数据拷贝
 * 2.校验报文头
 * ******************************************************/
int SyncCompute::read_modbus_packet(char *buf, int len)
{
    modbus_dev_t pd_dev;
    pd_dev.dev_addr = MODBUS_BC_ADDR;
    pd_dev.recv_len = 0;
    pd_dev.send_len = 0;
    for (int i = 0; i < len; i++) {
        pd_dev.recv_buf [pd_dev.recv_len++] = buf [i];
        if (pd_dev.recv_len == 1) {
            /* 装置地址为报文头 */
            if ((pd_dev.recv_buf [0] != pd_dev.dev_addr) &&
                    (pd_dev.recv_buf [0] != MODBUS_BC_ADDR)) {
                pd_dev.recv_len = 0;
            }
        }
    }
    return modbus_deal_msg ( &pd_dev );
}

/********************************************
 * modbus报文处理函数二
 * 1.校验报文长度
 * 2.根据功能码,分配处理接口
 * 3.如有需要,发送回复报文
 * *****************************************/
int SyncCompute::modbus_deal_msg(modbus_dev_t *ndp)
{
    int ret = -1;

    if (ndp->recv_len < 6) {        /* 判报文长度 */
        ndp->recv_len = 0;
        return -1;
    }

    unsigned char func_code = ndp->recv_buf [1];  /* 功能码 */

    switch(func_code) {
//    case MODBUS_FC_READ_REG:
//        ret = modbus_deal_read_reg (ndp);     //读一个或多个寄存器
//        break;
    case MODBUS_FC_WRITE_A_REG:
        ret = modbus_deal_write_a_reg (ndp);  //写一个寄存器
        break;
    case MODBUS_FC_WRITE_MORE_REG:            //写多个寄存器
        ret = modbus_deal_write_more_reg (ndp);
        break;
    default:
        ret = -1;
        break;
    }

    /* clear recv buf */
//    ndp->recv_len = 0;

    /* send message */
    if (ndp->send_len > 0) {
//        emit send_msg((char *)ndp->send_buf, ndp->send_len);
    }

    return ret;
}

/********************************************
 * modbus报文处理函数三(之一)
 * 1.校验报文是否为连接应答
 * 2.发送对应信息
 * *****************************************/
int SyncCompute::modbus_deal_write_a_reg(modbus_dev_t *ndp)
{
    if(ndp->recv_buf[2] == 0x00 && ndp->recv_buf[3] == 0x62
            && ndp->recv_buf[4] == 0x00 && ndp->recv_buf[5] == 0x00){
        emit get_reply();
    }
    return 0;
}

/********************************************
 * modbus报文处理函数三(之二)
 * 1.校验寄存器数,校验CRC
 * 2.根据附加功能码,分配处理接口
 * *****************************************/
int SyncCompute::modbus_deal_write_more_reg(modbus_dev_t *ndp)
{
    int ret = -1;
    unsigned char reg_num;
    unsigned short crc_recv, crc_calc;
    unsigned short fun_code;    //附加功能码

    reg_num = (ndp->recv_buf[4] << 8) + ndp->recv_buf[5];
    if((reg_num*2) != ndp->recv_buf[6]){
        return -1;
    }
    crc_recv = (ndp->recv_buf[reg_num*2 + 8] << 8) + ndp->recv_buf[reg_num*2 + 7];   //高位在前
    crc_calc = modbus_crc(ndp->recv_buf,reg_num*2+7);
    if (crc_calc != crc_recv) {
        printf ("crc error! %04x, %04x\n", crc_recv, crc_calc);
        return -1;
    }

    fun_code = (ndp->recv_buf[2] << 8)+ndp->recv_buf[3];
    switch(fun_code)
    {
    case md_rd_reg_sensor:          //GPS信息
        ret = modbus_deal_sensor_msg(ndp);
        break;
    case md_rd_reg_sync:            //同步信号
        ret = modbus_deal_sync_msg(ndp);
        break;
    default:
        break;
    }
    return ret;
}

unsigned short SyncCompute::modbus_crc(unsigned char *buf, unsigned char length)
{
    unsigned short val;
    unsigned short poly = 0xa001;
    unsigned char i, j;

    val = 0xffff;
    for (i = 0; i < length; i++) {
        val ^= buf [i];
        for (j = 0; j < 8; j++){
            if ((val & 0x0001) > 0) {
                val >>= 1;
                val ^= poly;
            }
            else {
                val >>= 1;
            }
        }
    }

    return val;
}

/******************************************************
 * 同步处理函数
 * 1.能够从报文中提取出通信延迟,频率,同步模式,计算出同步时间
 * 2.十次以内发送回复报文
 * 3.
 *
 * ***************************************************/
int SyncCompute::modbus_deal_sync_msg(modbus_dev_t *ndp)
{
    float group_num = ndp->recv_buf[9] * 0x1000000 + ndp->recv_buf[10] * 0x10000 + ndp->recv_buf[11] * 0x100  + ndp->recv_buf[12];          //组号
    float tel_delay = 0.100 * (ndp->recv_buf[13] * 0x1000000 + ndp->recv_buf[14] * 0x10000 + ndp->recv_buf[15] * 0x100  + ndp->recv_buf[16]); //延迟(ms)
    float freq = (ndp->recv_buf[7] * 0x100 + ndp->recv_buf[8]) / 100.0;         //频率(Hz)
    int sync_mode = ndp->recv_buf[17] * 0x100 + ndp->recv_buf[18];              //同步模式
    float sync_time = trans_time(last_time, tel_delay);                         //计算出同步时间

    qDebug()<<"sync mode:"<<sync_mode;

//    if(tel_delay > 20 && tel_delay < 45)
//        qDebug()<< QString::number(tel_delay,'f',1) << "\t" << QString::number(sync_time,'f',1);

//        r = qAbs(Compute::phase_error(sync_time, sync_time_list));
//        if(sync_time_list.count() == 8){
//            if( r < 1 ){     //判断输入值有效，进行同步
//                Common::time_addusec(last_time, -tel_delay * 1000);
//                send_sync(last_time.tv_sec, last_time.tv_usec);

//                qDebug()<<"success, error is:\t"<<r << "\tavrage is:\t" << Common::avrage(sync_time_list) << sync_time_list;
//            }
//            else{
//                qDebug()<<"failed, error is:\t"<<r << "\tavrage is:\t" << Common::avrage(sync_time_list) << sync_time_list;
//            }
//            qDebug()<<"\n";
//        }

//        sync_time_list.append(sync_time);
//        while(sync_time_list.count() > 8){
//            sync_time_list.removeFirst();
//        }

    struct timeval current_time;
    gettimeofday(&current_time, NULL);      //得到当次时间
    last_time = current_time;               //保存本次时间
    return 0;
}

int SyncCompute::modbus_deal_sensor_msg(modbus_dev_t *ndp)
{
    float wendu = (ndp->recv_buf[7] * 0x100 + ndp->recv_buf[8]) / 100.0;
    float shidu = (ndp->recv_buf[9] * 0x100 + ndp->recv_buf[10]) / 100.0;
    qDebug()<<"wen du:"<<wendu <<"C";
    qDebug()<<"shi du:"<<shidu << "%";

    QByteArray a;
    QList<QByteArray> gps;
    for (int i = 11; i < ndp->recv_buf[6] + 9; ++i) {
        a.append(QChar::fromLatin1(ndp->recv_buf[i]));
    }
    gps = a.split(',');
    GPSInfo gps_info(wendu,shidu,gps);
    emit get_gps_info(&gps_info);
    return 0;
}

float SyncCompute::trans_time(timeval recv_time, float delay_time)
{
    //    ms = last_time.tv_usec / 1000.0;                //上一次时间,转化为ms
    //    last_recv_time = ms - int(ms) + int(ms) % 20;   //保留小数点取余
//    sync_time = 200 + last_recv_time - tel_delay;
//    while (sync_time >= 20) {
//        sync_time -= 20;
//    }
    float ms = recv_time.tv_usec / 1000.0 - delay_time;
    if(ms < 0){
        ms += 1000;
    }
    ms = ms - int(ms) + int(ms) % 20;
    return ms;
}
