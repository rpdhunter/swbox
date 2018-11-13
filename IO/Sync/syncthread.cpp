#include "syncthread.h"
#include <QtDebug>
#include "Gui/Common/common.h"
#include "IO/Com/rdb/rdb.h"

SyncThread::SyncThread(QObject *parent) : QThread(parent)
{
    cpu_status = new CPUStatus;

    timer = new QTimer;
    timer->setInterval(1000);
    connect(timer,SIGNAL(timeout()), this,SLOT(write_to_rdb()));
    timer->start();

    change_thread_status();
}

//因为CPU信息是持续刷新的,而同步是选择性开启的,所以写入rdb的步骤放到了主线程里
void SyncThread::write_to_rdb()
{
    float temp, vcc;
    cpu_status->get_cpu_temp(&temp);
    cpu_status->get_cpu_vcc(&vcc);
    Common::rdb_set_yc_value(CPU_temp_yc, temp);
    Common::rdb_set_yc_value(CPU_vcc_yc, vcc);
}

void SyncThread::change_thread_status()
{
    if(sqlcfg->get_para()->sync_mode == SYNC_EXTERNAL){
        this->start();
    }
    else{
        this->terminate();
        this->wait();
    }
}

void SyncThread::run()
{
    qDebug()<<"SyncThread started!";

    while(1){
        get_current_sync_data();            //获取当前同步数据,存储到sync_data中

        double stand_T = 1000000.0 / sqlcfg->get_para()->freq_val;        //工频周期(us),50Hz时是20000us,60Hz时是16667us
        double stand_F = sqlcfg->get_para()->freq_val;

        //当存在6个有效数据时,进行过零点判断(过零点判断比较严格,必须单调)
        if(sync_data.length() > 6){
            if(sync_data.at(2).vcc < 0 && sync_data.at(3).vcc > 0
                    && sync_data.at(0).vcc < sync_data.at(1).vcc && sync_data.at(1).vcc < sync_data.at(2).vcc
                    && sync_data.at(3).vcc < sync_data.at(4).vcc && sync_data.at(4).vcc < sync_data.at(5).vcc){
                struct timeval zero_time = sync_data.at(5).t;
                //6组数据的时间间隔
                int time_interval = Common::time_interval(sync_data.at(0).t, sync_data.at(5).t);
                //算出过零点时刻
                int time_offset = time_interval * sync_data.at(5).vcc / (sync_data.at(5).vcc - sync_data.at(0).vcc);
                Common::time_addusec(zero_time, -time_offset);
                //存储过零点
                zero_times.append(zero_time);

                if(zero_times.count() == 15){        //校验15个周期
                    time_interval = Common::time_interval(zero_times.first(), zero_times.last() );
                    int a = round(time_interval / stand_T );    //取整
                    double f = a * 1000000.0 / time_interval;      //实际频率
                    if( 100 * (f - stand_F) / stand_F < 5){     //相对误差百分比
                        emit send_sync( (qint64)zero_times.last().tv_sec, (qint64)zero_times.last().tv_usec );
                        emit update_statusBar(tr("已同步,同步源:外,频率:%1Hz").arg(QString::number(f, 'f', 2)));
                        msleep(1000);           //循环休息1s

                        zero_times.clear();
                        sync_data.clear();

                    }
                    else{
                        zero_times.removeFirst();
                    }
                }
            }
            while(zero_times.count() > 1000){
                zero_times.removeFirst();
            }
            while(sync_data.count() > 6){
                sync_data.removeFirst();
            }
//            qDebug()<<"sync_data.count:"<<sync_data.count();
//            qDebug()<<"zero_times.count:"<<zero_times.count();
        }

        usleep(600);          //采样间隔为1ms
    }

    qDebug()<<"SyncThread exited!";
}

void SyncThread::get_current_sync_data()
{
    SYNC_DATA temp_data;
    cpu_status->get_sync_vvpn(&temp_data.vcc);
    gettimeofday( &temp_data.t, NULL );
    sync_data.append(temp_data);
//    qDebug()<<"sync = "<< temp_data.vcc << "mV\ttime = "<<temp_data.t.tv_usec;

}










