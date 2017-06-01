#include "recwave.h"
#include <QtDebug>
#include "IO/SqlCfg/sqlcfg.h"


RecWave::RecWave(G_PARA *gdata, MODE mode, QObject *parent) : QObject(parent)
{
    num =0;
    tdata = gdata;
    this->mode = mode;
    status = Free;

    timer = new QTimer;
    timer->setSingleShot(true);
    timer->setInterval(5000 );  //录5秒钟超声

    if(mode == TEV1){
        this->groupNum = 0;
    }
    else if(mode == AA_Ultrasonic){
        this->groupNum = 0x200;
    }


    connect(timer,SIGNAL(timeout()),this,SLOT(AA_rec_end()));

}

//从GUI发起录波指令
void RecWave::recStart (int time)
{	
    if (mode == TEV1) {
		tdata->set_send_para (sp_recstart, 1);
    }
    else if (mode == AA_Ultrasonic) {
		tdata->set_send_para (sp_recstart, 4);
		tdata->set_send_para (sp_tev_auto_rec, 0);
        timer->setInterval (time * 1000);
        qDebug()<<QString("going to rec wave time : %1 s").arg(time);
        timer->start();
    }
}

//上传数据的准备工作
void RecWave::startWork()
{
    _data.clear();
    time_start = QTime::currentTime();

    if (mode == TEV1) {
		tdata->set_send_para (sp_recstart, 0x10);		//数据上传开始
        this->groupNum = 0;
		tdata->set_send_para (sp_groupNum, 0);
    }
    else if (mode == AA_Ultrasonic) {
        num = 0;
        this->groupNum = 0x200;
		tdata->set_send_para (sp_groupNum, 0x200);
    }

}

//上传数据
void RecWave::work ()
{
	int i;
	
    if (mode == TEV1) {
        if (tdata->recv_para.groupNum == tdata->send_para.send_params [sp_groupNum].rval) {      //收发相匹配，拷贝数据
            for (i = 0; i < 256; i++) {
                _data.append((qint32)tdata->recv_para.recData [ i + 2 ] - 0x8000);
                //                qDebug("%08X",tdata->recv_para.recData [ i ]);
            }

            printf ("receive recWaveData! send_groupNum=%d\n", tdata->send_para.send_params [sp_groupNum].rval);

            this->groupNum++;
			tdata->set_send_para (sp_groupNum, groupNum);
        }
        else {                                                               //不匹配，再发一次
//            printf("receive recWaveData failed! send groupNum=%d\n",tdata->send_para.groupNum.rval);
//            printf("recv groupNum=%d\n",tdata->recv_para.groupNum);
			tdata->set_send_para (sp_groupNum, tdata->send_para.send_params [sp_groupNum].rval);
        }

        if (tdata->send_para.send_params [sp_groupNum].rval == GROUP_NUM_MAX) {        //接收组装数据完毕
			tdata->set_send_para (sp_recstart, 0);
            tdata->send_para.send_params [sp_groupNum].flag = false;

            qDebug()<<QString("rec wave cost time: %1 ms").arg( - QTime::currentTime().msecsTo(time_start));
            qDebug()<<"receive recWaveData complete! MODE = TEV";
            // 录波完成，发送数据，通知GUI和文件保存
            emit waveData (_data,mode);
            status = Free;
        }
    }
    else if (mode == AA_Ultrasonic) {

        if ((tdata->recv_para.groupNum + 0x200) == tdata->send_para.send_params [sp_groupNum].rval) {      //收发相匹配，拷贝数据
            for (i = 0; i < 256; i++) {
                _data.append((qint32)tdata->recv_para.recData [ i + 2 ]);
                num++;
            }


//            printf("receive recWaveData! send_groupNum=%d\n",tdata->send_para.groupNum.rval);

            ++groupNum;
			tdata->set_send_para (sp_groupNum, groupNum);
        }
        else {                                                               //不匹配，再发一次
            printf("receive recWaveData failed! send groupNum=%d\n",tdata->send_para.send_params [sp_groupNum].rval);
            printf("recv groupNum=%d\n",tdata->recv_para.groupNum);
			tdata->set_send_para (sp_groupNum, tdata->send_para.send_params [sp_groupNum].rval);
        }

        if (tdata->send_para.send_params [sp_groupNum].rval == (0x200 + GROUP_NUM_MAX)){
            this->groupNum = 0x200;
            //qDebug()<<"4096points complete!";
			tdata->set_send_para (sp_groupNum, groupNum);
        }
    }

}

void RecWave::AA_rec_end()
{
	tdata->set_send_para (sp_groupNum, 0);
	tdata->set_send_para (sp_recstart, 0);

    qDebug()<<"receive recAAData complete! MODE = AA Ultrasonic" << _data.length() << " points";
    //录波完成，发送数据，通知GUI和文件保存
    emit waveData(_data,mode);

	tdata->set_send_para (sp_tev_auto_rec, sqlcfg->get_para()->tev1_sql.auto_rec);	//将自动录波替换为设置值

    status = Free;
}

