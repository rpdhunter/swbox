#include "optionexpert.h"
#include "ui_optionexpert.h"
#include "Gui/Common/common.h"

OptionExpert::OptionExpert(SQL_PARA *sql, QWidget *parent) :
    TabWidget(sql,parent),
    ui(new Ui::OptionExpert)
{
    ui->setupUi(this);
    this->resize(TABWIDGET_X,TABWIDGET_Y);



    sync_socket = new SyncSocket;
    connect(sync_socket, SIGNAL(update_status(QString)), ui->label_status, SLOT(setText(QString)));
    connect(sync_socket, SIGNAL(update_status(QString)), this, SLOT(fresh()));
    connect(sync_socket, SIGNAL(send_gps_info(GPSInfo*)), this, SLOT(deal_gps_info(GPSInfo*)));     //GPS信息
    connect(sync_socket, SIGNAL(send_freq(float)), this, SIGNAL(fregChanged(float)) );              //频率变化
    connect(sync_socket, SIGNAL(send_sync(qint64,qint64)), this, SIGNAL(send_sync(qint64,qint64)), Qt::DirectConnection);   //同步时间
    connect(this, SIGNAL(set_sync_source(int)), sync_socket, SLOT(set_sync_source(int)));           //修改同步源

    thread = new QThread;
    sync_socket->moveToThread(thread);
    thread->start(QThread::TimeCriticalPriority);


    _sync_clamp = new SyncClamp;

    fresh();
}

OptionExpert::~OptionExpert()
{
    delete ui;
}

void OptionExpert::update_status(QString str)
{
    ui->label_status->setText(str);
}

void OptionExpert::work()
{
    row = 1;
    fresh();
}

void OptionExpert::do_key_ok()
{
//    row = 0;
    set_sync_status();
    fresh();
    emit save();
//    emit quit();
}

void OptionExpert::do_key_cancel()
{
    row = 0;
    *sql_para = *sqlcfg->get_para();
    fresh();
    emit quit();
}

void OptionExpert::do_key_up_down(int d)
{
    Common::change_index(row,d,5,1);
    fresh();
}

void OptionExpert::do_key_left_right(int d)
{
    QList<int> list;
    list << sync_none << sync_clamp_internal << sync_vac220_110 <<  sync_light << sync_clamp;
    switch (row) {
    case 1:         //录波文件数
        Common::change_index(sql_para->max_rec_num, d*100, 5000, 100);
        break;
    case 2:         //录波间隔
        Common::change_index(sql_para->auto_rec_interval, d, 10, 1);
        break;
    case 3:         //蜂鸣器报警
        sql_para->buzzer_on = !sql_para->buzzer_on;
        break;
    case 4:         //同步模式
        Common::change_index(sql_para->sync_mode, d, list);
        break;
    case 5:         //同步补偿值
        Common::change_index(sql_para->sync_val, d, 180, -180);
        break;
    default:
        break;
    }

    fresh();
}

void OptionExpert::set_sync_status()
{
    emit set_sync_source(sql_para->sync_mode);       //设置同步源
    if(sql_para->sync_mode == sync_none){
        emit fregChanged(50);
    }

}

void OptionExpert::fresh()
{
    ui->lineEdit_MaxFileNum->setText(QString::number(sql_para->max_rec_num));
    ui->lineEdit_interval->setText(QString::number(sql_para->auto_rec_interval));
    ui->lineEdit_sync_justify->setText(QString::number(sql_para->sync_val));

    switch (sql_para->sync_mode) {
    case sync_light:
        ui->rbt_sync_light->setChecked(true);
        break;
    case sync_vac220_110:
        ui->rbt_sync_vac220_110->setChecked(true);
        break;
    case sync_clamp:
        ui->rbt_sync_clamp->setChecked(true);
        break;
    case sync_clamp_internal:
        ui->rbt_sync_clamp_internal->setChecked(true);
        break;
    default:
        ui->rbt_sync_none->setChecked(true);
        break;
    }
    ui->checkBox_buzzer->setChecked(sql_para->buzzer_on);

    ui->lab_MaxRec->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_Rec_Interval->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_buzzer->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_SYNC->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_SYNC_justify->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lineEdit_MaxFileNum->deselect();
    ui->lineEdit_interval->deselect();
    ui->lineEdit_sync_justify->deselect();

    switch (row) {
    case 1:
        ui->lab_MaxRec->setStyleSheet("QLabel{color:#0EC3FD;}");
        ui->lineEdit_MaxFileNum->selectAll();
        break;
    case 2:
        ui->lab_Rec_Interval->setStyleSheet("QLabel{color:#0EC3FD;}");
        ui->lineEdit_interval->selectAll();
        break;
    case 3:
        ui->lab_buzzer->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 4:
        ui->lab_SYNC->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 5:
        ui->lab_SYNC_justify->setStyleSheet("QLabel{color:#0EC3FD;}");
        ui->lineEdit_sync_justify->selectAll();
        break;
    default:
        break;
    }

    if(ui->label_status->text().contains(tr("已连接"))){
        ui->label_status->setStyleSheet("QLabel{color:green}");
        sqlcfg->get_global()->wifi_working = true;
    }
    else{
        ui->label_status->setStyleSheet("QLabel{color:red}");
    }
}

void OptionExpert::deal_gps_info(GPSInfo *gps)
{
//    ui->label_gps->setText(tr("GPS信息:") + gps->gps_str());
    if(gps != NULL){
//        qDebug()<<"deal_gps_info"<<gps->gps_str();

        emit gpsChanged(gps->gps_str());
    }
    else{
        qDebug()<<"deal_gps_info error";
    }

}

