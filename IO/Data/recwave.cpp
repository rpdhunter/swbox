#include "recwave.h"
#include <QtDebug>
#include "IO/SqlCfg/sqlcfg.h"


RecWave::RecWave(G_PARA *gdata, MODE mode, QObject *parent) : QObject(parent)
{
    tdata = gdata;
    this->mode = mode;
    status = Free;

    switch (mode) {
    case TEV1:
        groupNum_Offset = 0;
        break;
    case TEV2:
        groupNum_Offset = 0x100;
        break;
    case AA_Ultrasonic:
        groupNum_Offset = 0x200;
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
void RecWave::recStart (int time)
{	
    switch (mode) {
    case TEV1:
        tdata->set_send_para (sp_recstart_ad1, 1);
        break;
    case TEV2:
        tdata->set_send_para (sp_recstart_ad2, 1);
        break;
    case AA_Ultrasonic:
        tdata->set_send_para (sp_recstart_ad3, 1);
        tdata->set_send_para (sp_tev_auto_rec, 0);
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
    time_start = QTime::currentTime();

    switch (mode) {
    case TEV1:
        tdata->set_send_para (sp_recstart_ad1, 2);		//数据上传开始
        break;
    case TEV2:
        tdata->set_send_para (sp_recstart_ad2, 2);		//数据上传开始
        break;
    case AA_Ultrasonic:
        break;
    default:
        break;
    }
    this->groupNum = 0;
    tdata->set_send_para (sp_groupNum, groupNum_Offset);

}

//上传数据
void RecWave::work ()
{
    int i;

    switch (mode) {
    case TEV1:
        if (tdata->recv_para.groupNum + groupNum_Offset == tdata->send_para.send_params [sp_groupNum].rval) {      //收发相匹配，拷贝数据
            for (i = 0; i < 256; i++) {
                _data.append((qint32)tdata->recv_para.recData [ i + 2 ] - 0x8000);
            }
//            printf ("receive recWaveData! send_groupNum=%d\n", tdata->send_para.send_params [sp_groupNum].rval);
            groupNum++;
            tdata->set_send_para (sp_groupNum, groupNum + groupNum_Offset);
        }
        else {                                                               //不匹配，再发一次
            printf("receive recWaveData failed! send groupNum=%d\n",tdata->send_para.send_params [sp_groupNum].rval);
            printf("recv groupNum=%d\n",tdata->recv_para.groupNum);
            tdata->set_send_para (sp_groupNum, tdata->send_para.send_params [sp_groupNum].rval);
        }

        //结束判断
        if (groupNum == GROUP_NUM_MAX) {        //接收组装数据完毕
            tdata->set_send_para (sp_recstart_ad1, 0);
            tdata->send_para.send_params [sp_groupNum].flag = false;

            qDebug()<<QString("rec wave cost time: %1 ms").arg( - QTime::currentTime().msecsTo(time_start));
            qDebug()<<"receive recWaveData complete! MODE = TEV1";
            // 录波完成，发送数据，通知GUI和文件保存
            emit waveData (_data,mode);
            status = Free;
        }
        break;
    case TEV2:
        if (tdata->recv_para.groupNum + groupNum_Offset == tdata->send_para.send_params [sp_groupNum].rval) {      //收发相匹配，拷贝数据
            for (i = 0; i < 256; i++) {
                _data.append((qint32)tdata->recv_para.recData [ i + 2 ] - 0x8000);
            }
            printf ("receive recWaveData! send_groupNum=%d\n", tdata->send_para.send_params [sp_groupNum].rval);
            groupNum++;
            tdata->set_send_para (sp_groupNum, groupNum + groupNum_Offset);
        }
        else {                                                               //不匹配，再发一次
            printf("receive recWaveData failed! send groupNum=%d\n",tdata->send_para.send_params [sp_groupNum].rval);
            printf("recv groupNum=%d\n",tdata->recv_para.groupNum);
            tdata->set_send_para (sp_groupNum, tdata->send_para.send_params [sp_groupNum].rval);
        }

        //结束判断
        if (groupNum == GROUP_NUM_MAX) {        //接收组装数据完毕
            tdata->set_send_para (sp_recstart_ad2, 0);
            tdata->send_para.send_params [sp_groupNum].flag = false;

            qDebug()<<QString("rec wave cost time: %1 ms").arg( - QTime::currentTime().msecsTo(time_start));
            qDebug()<<"receive recWaveData complete! MODE = TEV2";
            // 录波完成，发送数据，通知GUI和文件保存
            emit waveData (_data,mode);
            status = Free;
        }
        break;
    case AA_Ultrasonic:
        if ((tdata->recv_para.groupNum + groupNum_Offset) == tdata->send_para.send_params [sp_groupNum].rval) {      //收发相匹配，拷贝数据
            for (i = 0; i < 256; i++) {
                _data.append((qint32)tdata->recv_para.recData [ i + 2 ]);
            }
//            printf("receive recWaveData! send_groupNum=%d\n",tdata->send_para.groupNum.rval);

            groupNum++;
            if (groupNum == GROUP_NUM_MAX){         //接受完16组数据，重新开始
                groupNum = 0;
            }
            tdata->set_send_para (sp_groupNum, groupNum + groupNum_Offset);
        }
        else {                                                               //不匹配，再发一次
            printf("receive recWaveData failed! send groupNum=%d\n",tdata->send_para.send_params [sp_groupNum].rval);
            printf("recv groupNum=%d\n",tdata->recv_para.groupNum);
            tdata->set_send_para (sp_groupNum, tdata->send_para.send_params [sp_groupNum].rval);
        }
        break;
    default:
        break;
    }

}

void RecWave::AA_rec_end()
{
	tdata->set_send_para (sp_groupNum, 0);
    tdata->set_send_para (sp_recstart_ad3, 0);

    qDebug()<<"receive recAAData complete! MODE = AA Ultrasonic" << _data.length() << " points";
    //录波完成，发送数据，通知GUI和文件保存
    emit waveData(_data,mode);

	tdata->set_send_para (sp_tev_auto_rec, sqlcfg->get_para()->tev1_sql.auto_rec);	//将自动录波替换为设置值

    status = Free;
}

