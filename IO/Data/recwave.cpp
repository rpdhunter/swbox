#include "recwave.h"
#include <QtDebug>
#include "IO/SqlCfg/sqlcfg.h"
#include "Gui/Common/common.h"

#define REC_NUM             500
#define GROUP_NUM_MAX       8              //组号最大值


RecWave::RecWave(G_PARA *gdata, CHANNEL channel, QObject *parent) : QObject(parent)
{
    data = gdata;
    this->channel = channel;        //channel初始化后不可更改
    status = Free;

    switch (channel) {
    case CHANNEL_H1:
        groupNum_Offset = 0x100;
        mode = (MODE)sqlcfg->get_para()->menu_h1;
        break;
    case CHANNEL_H2:
        groupNum_Offset = 0x200;
        mode = (MODE)sqlcfg->get_para()->menu_h2;
        break;
    case CHANNEL_L1:
        groupNum_Offset = 0x400;
        mode = (MODE)sqlcfg->get_para()->menu_l1;
        break;
    case CHANNEL_L2:
        groupNum_Offset = 0x800;
        mode = (MODE)sqlcfg->get_para()->menu_l2;
        break;
    default:
        groupNum_Offset = 0;
        break;
    }

    timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this,SLOT(l_channel_rec_end()));
}

//从GUI发起录波指令
void RecWave::recStart (MODE , int time)
{
//    mode = m;
    switch (channel) {
    case CHANNEL_H1:
        data->set_send_para (sp_rec_start_h1, 1);
        break;
    case CHANNEL_H2:
        data->set_send_para (sp_rec_start_h2, 1);
        break;
    case CHANNEL_L1:
        data->set_send_para (sp_rec_start_l1, 1);
        data->set_send_para (sp_auto_rec, 0);
        timer->setInterval (time * 1000);
        qDebug()<<QString("AA1 is going to rec wave time : %1 s").arg(time);
        timer->start();
        break;
    case CHANNEL_L2:
        data->set_send_para (sp_rec_start_l2, 1);
        data->set_send_para (sp_auto_rec, 0);
        timer->setInterval (time * 1000);
        qDebug()<<QString("AA2 is going to rec wave time : %1 s").arg(time);
        timer->start();
        break;
    default:
        break;
    }
}

//上传数据的准备工作
void RecWave::startWork()
{
    _data.clear();
//    time_start = QTime::currentTime();

    switch (channel) {
    case CHANNEL_H1:
        data->set_send_para (sp_rec_start_h1, 2);		//数据上传开始
        break;
    case CHANNEL_H2:
        data->set_send_para (sp_rec_start_h2, 2);		//数据上传开始
        break;
    default:
        break;
    }
    this->groupNum = 0;
    data->set_send_para (sp_group_num, groupNum_Offset);

}

//上传数据
void RecWave::work ()
{
    if(channel == CHANNEL_L1 || channel == CHANNEL_L2){
        l_channel_rec_core();
    }
    else{
        h_channel_rec_core();
    }

}

void RecWave::l_channel_rec_end()
{
    data->set_send_para(sp_group_num, 0);
    if(channel == CHANNEL_L1){
        data->set_send_para(sp_rec_start_l1, 0);
    }
    else if(channel == CHANNEL_L2){
        data->set_send_para(sp_rec_start_l2, 0);
    }

    qDebug()<<"receive CHANNEL_LOW complete!" << _data.length() << " points";
    //录波完成，发送数据，通知GUI和文件保存
    emit waveData(_data,mode);

    data->set_send_para(sp_auto_rec, sqlcfg->get_para()->tev1_sql.auto_rec);	//将自动录波替换为设置值
    status = Free;
}

void RecWave::h_channel_rec_core()
{
    if (data->recv_para_rec.groupNum + groupNum_Offset == data->send_para.send_params [sp_group_num].rval) {      //收发相匹配，拷贝数据
        for (int i = 0; i < REC_NUM; i++) {
            _data.append((qint32)data->recv_para_rec.data[ i + 2 ] - 0x8000 - Common::offset_zero_code(mode));      //录波时就减掉了偏置
        }
//        qDebug()<<"h_channel receive data succeed! send groupNum = "<<data->send_para.send_params [sp_group_num].rval;
        groupNum++;
        data->set_send_para (sp_group_num, groupNum + groupNum_Offset);
    }
    else {                                                               //不匹配，再发一次
        qDebug()<<"h_channel receive failed! send groupNum = "<<data->send_para.send_params [sp_group_num].rval
               << "recv groupNum = "<< data->recv_para_rec.groupNum + groupNum_Offset;
        data->set_send_para (sp_group_num, groupNum + groupNum_Offset);
    }

    //结束判断
    if (groupNum == GROUP_NUM_MAX) {        //接收组装数据完毕
        switch (channel) {
        case CHANNEL_H1:
            data->set_send_para (sp_rec_start_h1, 0);		//数据上传开始
            break;
        case CHANNEL_H2:
            data->set_send_para (sp_rec_start_h2, 0);		//数据上传开始
            break;
        default:
            break;
        }
//        qDebug()<<QString("rec wave cost time: %1 ms").arg( - QTime::currentTime().msecsTo(time_start));
//        qDebug()<<"receive recWaveData complete! MODE = "<< mode << "points = "<< _data.length();
        // 录波完成，发送数据，通知GUI和文件保存
        emit waveData (_data,mode);
        status = Free;
    }
}

void RecWave::l_channel_rec_core()
{
    if ((data->recv_para_rec.groupNum + groupNum_Offset) == data->send_para.send_params [sp_group_num].rval) {      //收发相匹配，拷贝数据
        for (int i = 0; i < 256; i++) {
            _data.append((qint32)data->recv_para_rec.data[ i + 2 ]);
//            _data.append((qint32)data->recv_para_rec.data[ i + 2 ] - 0x8000);

        }
//        printf("receive recWaveData! send_groupNum=%d\n",data->recv_para_rec.groupNum );

        groupNum++;
        if (groupNum == 16){         //接受完16组数据，重新开始
            groupNum = 0;
        }
        data->set_send_para (sp_group_num, groupNum + groupNum_Offset);
    }
    else {                                                               //不匹配，再发一次
        qDebug()<<"AA_Ultrasonic receive failed! send groupNum = "<<data->send_para.send_params [sp_group_num].rval
               << "recv groupNum = "<< data->recv_para_rec.groupNum + groupNum_Offset;
        data->set_send_para (sp_group_num, groupNum + groupNum_Offset);
    }
}

