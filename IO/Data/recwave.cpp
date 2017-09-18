#include "recwave.h"
#include <QtDebug>
#include "IO/SqlCfg/sqlcfg.h"

#define REC_NUM             500
#define GROUP_NUM_MAX       8              //组号最大值


RecWave::RecWave(G_PARA *gdata, MODE mode, QObject *parent) : QObject(parent)
{
    data = gdata;
    this->mode = mode;
    status = Free;

    switch (mode) {
    case TEV1:
        groupNum_Offset = 0x100;
        break;
    case TEV2:
        groupNum_Offset = 0x200;
        break;
    case AA_Ultrasonic:
        groupNum_Offset = 0x1000;
        break;
    case HFCT1:
        groupNum_Offset = 0x400;
        break;
    case HFCT2:
        groupNum_Offset = 0x800;
        break;
    default:
        break;
    }

    timer = new QTimer;
    timer->setSingleShot(true);
//    timer->setInterval(5000 );  //录5秒钟超声
    connect(timer,SIGNAL(timeout()),this,SLOT(AA_rec_end()));
}

//从GUI发起录波指令
void RecWave::recStart (MODE m, int time)
{
    mode = m;
    switch (mode) {
    case TEV1:
        data->set_send_para (sp_rec_start_tev1, 1);
        break;
    case TEV2:
        data->set_send_para (sp_rec_start_tev2, 1);
        break;
    case HFCT1:
        data->set_send_para (sp_rec_start_hfct1, 1);
        break;
    case HFCT2:
        data->set_send_para (sp_rec_start_hfct2, 1);
        break;
//    case HFCT_CONTINUOUS:
//        data->set_send_para (sp_recstart_ad2, 1);
//        break;
    case AA_Ultrasonic:
        data->set_send_para (sp_rec_start_aa1, 1);
        data->set_send_para (sp_auto_rec, 0);
        timer->setInterval (time * 1000);
        qDebug()<<QString("going to rec wave time : %1 s").arg(time);
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

    switch (mode) {
    case TEV1:
        data->set_send_para (sp_rec_start_tev1, 2);		//数据上传开始
        break;
    case TEV2:
        data->set_send_para (sp_rec_start_tev2, 2);		//数据上传开始
        break;
    case HFCT1:
        data->set_send_para (sp_rec_start_hfct1, 2);		//数据上传开始
        break;
    case HFCT2:
        data->set_send_para (sp_rec_start_hfct2, 2);		//数据上传开始
        break;
//    case HFCT_CONTINUOUS:
//        data->set_send_para (sp_recstart_ad2, 2);		//数据上传开始
//        break;
    case AA_Ultrasonic:
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
    if(mode == AA_Ultrasonic){
        l_channel_rec_core();
    }
    else{
        h_channel_rec_core();
    }

}

void RecWave::AA_rec_end()
{
    data->set_send_para(sp_group_num, 0);
    data->set_send_para(sp_rec_start_aa1, 0);

    qDebug()<<"receive recAAData complete! MODE = AA Ultrasonic" << _data.length() << " points";
    //录波完成，发送数据，通知GUI和文件保存
    emit waveData(_data,mode);

    data->set_send_para(sp_auto_rec, sqlcfg->get_para()->tev1_sql.auto_rec);	//将自动录波替换为设置值

    status = Free;
}

void RecWave::h_channel_rec_core()
{
    if (data->recv_para_rec.groupNum + groupNum_Offset == data->send_para.send_params [sp_group_num].rval) {      //收发相匹配，拷贝数据
        for (int i = 0; i < REC_NUM; i++) {
            _data.append((qint32)data->recv_para_rec.data[ i + 2 ] - 0x8000);
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
        switch (mode) {
        case TEV1:
            data->set_send_para (sp_rec_start_tev1, 0);		//数据上传开始
            break;
        case TEV2:
            data->set_send_para (sp_rec_start_tev2, 0);		//数据上传开始
            break;
        case HFCT1:
            data->set_send_para (sp_rec_start_hfct1, 0);		//数据上传开始
            break;
        case HFCT2:
            data->set_send_para (sp_rec_start_hfct2, 0);		//数据上传开始
            break;
        default:
            break;
        }
//        qDebug()<<QString("rec wave cost time: %1 ms").arg( - QTime::currentTime().msecsTo(time_start));
//        qDebug()<<"receive recWaveData complete! MODE = "<< mode << "points = "<< _data.length();
        // 录波完成，发送数据，通知GUI和文件保存
        emit waveData (_data,mode);
//            qDebug()<<"TEV1 Working --> Free";
        status = Free;
    }
}

void RecWave::l_channel_rec_core()
{
    if ((data->recv_para_rec.groupNum + groupNum_Offset) == data->send_para.send_params [sp_group_num].rval) {      //收发相匹配，拷贝数据
        for (int i = 0; i < 256; i++) {
            _data.append((qint32)data->recv_para_rec.data[ i + 2 ]);
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

