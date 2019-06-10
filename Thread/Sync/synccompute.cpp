#include "synccompute.h"
#include <QThread>

/*********************************************************
 * modbus报文处理类,主要处理3类报文
 * 1.GPS报文
 * 2.同步报文
 * 3.保活应答报文
 * ******************************************************/
SyncCompute::SyncCompute(QObject *parent) : QObject(parent)
{
    gps_info = new GPSInfo(this);
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

void SyncCompute::show_time(QString str)
{
    struct timeval current_time;
    gettimeofday(&current_time, NULL);      //得到当次时间
    qDebug()<<str + " time:"<<current_time.tv_usec;
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
    if(group_num < 10){              //不满10次,发送回复
        emit get_sync();
    }

//    qDebug()<<"group num :"<<group_num;

    float tel_delay = 0.100 * (ndp->recv_buf[13] * 0x1000000 + ndp->recv_buf[14] * 0x10000 + ndp->recv_buf[15] * 0x100  + ndp->recv_buf[16]); //延迟(ms)
    float freq = (ndp->recv_buf[7] * 0x100 + ndp->recv_buf[8]) / 100.0;         //频率(Hz)

    float sync_time = trans_time(last_time, tel_delay);                         //计算出同步时间
    if(group_num > 0){                              //第0次由于延迟还没有计算，计算出的时间是无意义的
        timeval zero_time;
        zero_time.tv_sec = last_time.tv_sec;
        zero_time.tv_usec = last_time.tv_usec - tel_delay * 1000;
        sync_map.insert(sync_time, zero_time);
    }
    else if(group_num == 0){
        sync_map.clear();
        emit send_freq(freq);       //发送收到的第一个频率
    }

//    int sync_mode = ndp->recv_buf[17] * 0x100 + ndp->recv_buf[18];              //同步模式
//    qDebug()<<"sync mode:"<<sync_mode;
    qDebug()<<"group_num:"<<group_num<<"\ttel_delay:"<<tel_delay << "\tfreq:"<<freq << "\tsync_time:"<< sync_time;

    if(group_num >= 10 && sync_map.count() >= 10){                    //一秒内的第十次数据,就开始结算之前的数据
        timeval zero_time = compute_zero_time();
        emit send_sync(zero_time.tv_sec, zero_time.tv_usec);
//        emit send_freq(freq);
        sync_map.clear();
        qDebug()<<"--------------------end-----------------";
    }

    struct timeval current_time;
    gettimeofday(&current_time, NULL);      //得到当次时间
    last_time = current_time;               //保存本次时间
    return 0;
}

int SyncCompute::modbus_deal_sensor_msg(modbus_dev_t *ndp)
{
    float wendu = (ndp->recv_buf[7] * 0x100 + ndp->recv_buf[8]) / 100.0;
    float shidu = (ndp->recv_buf[9] * 0x100 + ndp->recv_buf[10]) / 100.0;
    qDebug()<<"Temp:"<<wendu <<"C\tHumi:"<<shidu << "%";

    QByteArray a;
    QList<QByteArray> gps;
    for (int i = 11; i < ndp->recv_buf[6] + 9; ++i) {
        a.append(QChar::fromLatin1(ndp->recv_buf[i]));
    }
    gps = a.split(',');
    gps_info->init(wendu,shidu,gps);
    emit send_gps_info(gps_info);

//    temp_list.append(wendu);
//    if(temp_list.count() > 4){
//        float del = temp_list.last() - temp_list.first();      //温度变化值
//        QVector<float> temp_table;                          //定义温度表,通过查表确定温度修正值
//        temp_table << 17 << 16 << 15 << 14 << 13            //0.1, 0.3, 0.5, 0.7, 0.9
//                   << 12.5 << 12 << 11.5 << 11.25 << 11     //1.1, 1.3, 1.5, 1.7, 1.9
//                   << 10.75 << 10.5;                        //2.1, 2.3
//        int n = qRound( (del - 0.1) / 0.2 );        //四舍五入取整
//        float modify;
//        if(n < 0){
//            modify = 18;
//        }
//        else if(n < temp_table.count()){
//            modify = temp_table.at(n);
//        }
//        else{
//            modify = 10;
//        }

//        wendu -= modify;                  //温度加入动态修正
//        qDebug()<<"del:"<<del<<"\tTemp modify:"<<modify<<"\tTemp:"<<wendu;


//        temp_list.removeFirst();

//        QByteArray a;
//        QList<QByteArray> gps;
//        for (int i = 11; i < ndp->recv_buf[6] + 9; ++i) {
//            a.append(QChar::fromLatin1(ndp->recv_buf[i]));
//        }
//        gps = a.split(',');
//        gps_info->init(wendu,shidu,gps);
//        emit send_gps_info(gps_info);
//    }

    return 0;
}

/****************************************************************
 * 根据接收时间和通信延迟计算出发送时间，并转化成【0-20）ms内的标准数据
 * tobe:60Hz情况需重做
 * *************************************************************/
float SyncCompute::trans_time(timeval recv_time, float delay_time)
{
    float ms = recv_time.tv_usec / 1000.0 - delay_time;
    if(ms < 0){
        ms += 1000;
    }
    ms = ms - int(ms) + int(ms) % 20;
    return ms;
}

float SyncCompute::compute_sync_time(QVector<float> list)
{
    //输入数据量检查
    if(list.count() < 5){
        qDebug()<<"compute_sync_time input error";
        return 0;
    }

    //判断序列是否处在临界位置
    int n = 0;
    foreach (float l, list) {
        if(l < 3 || l > 17){
            n++;
        }
    }
    if(n > 5){      //处于临界位置，需要数据映射处理
        float temp;
        for (int i = 0; i < list.count(); ++i) {    //将数据做一个映射【0-20)=> 【5-25)，以保证序列均值不会出现在区间边界
            temp = list.at(i) + 10;
            if(temp > 25){
                temp -= 20;
            }
            list[i] = temp;
        }
    }

    //格拉布斯准则（先用简化版）
    qSort(list.begin(), list.end());                //排序
    list.removeFirst();
    list.removeFirst();
    list.removeLast();
    list.removeLast();
    float av = Compute::avrage(list);


    if(n > 5){                              //映射回正常范围
        av -= 10;
        if(av < 0){
            av += 20;
        }
    }


//    qDebug()<<list;
    qDebug()<<" [ "<<av<<" ] ";

    return av;
}

float SyncCompute::compute_delay_time(QVector<float> list)
{
    //输入数据量检查
    if(list.count() < 5){
        qDebug()<<"compute_sync_time input error";
        return 0;
    }

    //格拉布斯准则（先用简化版）
    qSort(list.begin(), list.end());                //排序
    list.removeFirst();
    list.removeFirst();
    list.removeLast();
    list.removeLast();
    float av = Compute::avrage(list);

    qDebug()<<" [ "<<av<<" ] ";

    return av;
}

timeval SyncCompute::compute_zero_time()
{
    QList<float> list = sync_map.keys();

    //判断序列是否处在临界位置
    int n = 0;
    foreach (float l, list) {
        if(l < 3 || l > 17){
            n++;
        }
    }
    if(n > 5){      //处于临界位置，需要数据映射处理
        QMap<float, timeval> new_map;
        QMapIterator<float, timeval> i(sync_map);
        float temp_key;
        while (i.hasNext()) {
            i.next();
            temp_key = i.key() + 10;
            if(temp_key > 25){
                temp_key -= 20;
            }
            new_map.insert(temp_key, i.value());
        }
        return compute_zero_time(new_map);
    }
    else{
        return compute_zero_time(sync_map);
    }
}

timeval SyncCompute::compute_zero_time(QMap<float, timeval> map)
{
    float key = map.keys().at(5);
    qDebug()<<map.keys();
    return map.value(key);
}

















