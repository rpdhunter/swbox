#include "statusbar.h"
#include "ui_mainwindow.h"
#include "Common/common.h"
#include <QDate>
#include <QTime>

#define LOW_POWER_TIMES     20
#define MISS_SYNC_TIMES     10          //SYNC失步时间
#define MISS_GPS_TIMES      35          //GPS失步时间

StatusBar::StatusBar(Ui::MainWindow *_ui, QObject *parent) :
    QObject(parent),
    ui(_ui)
{
    battery = new Battery;
    low_power = LOW_POWER_TIMES;

    timer_time = new QTimer();
    timer_time->setInterval(1000);   //1秒1跳
    timer_time->start();

    timer_batt = new QTimer();
    timer_batt->setInterval(10000);   //10秒1跳
    timer_batt->start();

    timer_message = new QTimer();
    timer_message->setSingleShot(true);
    timer_message->setInterval(2000);       //2秒后恢复显示标准信息

    timer_miss_sync = new QTimer();
    timer_miss_sync->setSingleShot(true);

    timer_miss_gps = new QTimer();
    timer_miss_gps->setSingleShot(true);

//    set_reboot_time();

    connect(timer_time, SIGNAL(timeout()), this, SLOT(fresh_status()) );
//    connect(timer_batt, SIGNAL(timeout()), this, SLOT(fresh_batt()) );
    connect(timer_time, SIGNAL(timeout()), this, SLOT(fresh_batt()) );
    connect(timer_message, SIGNAL(timeout()), this, SLOT(message_reset()) );
    connect(timer_miss_sync, SIGNAL(timeout()), this, SLOT(fresh_miss_freq()) );
    connect(timer_miss_gps, SIGNAL(timeout()), this, SLOT(fresh_miss_gps()) );

    if(sqlcfg->get_para()->menu_asset == Disable){          //没配置资产，则状态栏资产图标隐藏
        ui->lab_asset->hide();
    }

    int w = 270;
    ui->lab_temp->setGeometry(RESOLUTION_X - w - 5 ,250,w,20);

//    ui->lab_imformation->set

    freq = 0;
}

void StatusBar::fresh_status()
{
    ui->lab_time->setText(QDate::currentDate().toString(tr("yyyy年M月d日"))
                          + " "
                          + QTime::currentTime().toString("h:mm:ss"));

//    int s = timer_reboot->remainingTime() / 1000;    //自动关机秒数
//    if(timer_reboot->isActive() && s < 60){
//        ui->lab_imformation->setText(tr("再过%1秒将自动关机，按任意键取消").arg(s));
//    }
    if(freq == 0){
        ui->lab_freq->setStyleSheet("QLabel {color:white;}");
        ui->lab_freq->setText(QString("%1Hz").arg(sqlcfg->get_para()->freq_val));
    }
    else{
//        qDebug()<<"MainWindow::fresh_status():"<<freq;
//        ui->lab_freq->setStyleSheet("QLabel {color:green;}");
//        ui->lab_freq->setText(QString("%1Hz").arg(QString::number(freq, 'f', 2)) );
    }

    if(!gps_info.isEmpty()){
        ui->lab_temp->show();
        ui->lab_temp->setText(gps_info);
    }
    else{
        ui->lab_temp->hide();
    }


}

void StatusBar::fresh_batt()
{
    //自动关机
    if(battery->is_low_power()){
        low_power--;
        qDebug()<<"low_power = "<<low_power;
        if(low_power == 0){
            emit system_reboot();
        }
    }
    else{
        low_power = LOW_POWER_TIMES;      //检测错误,重置
    }

    //电量显示
    int batt_val = battery->battPercentValue();

    if(batt_val < 0){
        ui->lab_pwr_num->setText("");
        return;
    }


    //UI
    ui->lab_pwr_num->setText(QString("%1%").arg(batt_val));
    if(batt_val>25){
        ui->lab_pwr_num->setStyleSheet("QLabel {color:white;}");
    }
    else{
        ui->lab_pwr_num->setStyleSheet("QLabel {color:red;}");
    }

//    if(battery->is_charging()){
//        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr100.png);}");        //充电图标
//        return;
//    }

    switch (batt_val / 10) {
    case 0:
        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr0.png);}");
        break;
    case 1:
        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr10.png);}");
        break;
    case 2:
        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr20.png);}");
        break;
    case 3:
        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr30.png);}");
        break;
    case 4:
        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr40.png);}");
        break;
    case 5:
        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr50.png);}");
        break;
    case 6:
        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr60.png);}");
        break;
    case 7:
        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr70.png);}");
        break;
    case 8:
        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr80.png);}");
        break;
    case 9:
        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr90.png);}");
        break;
    case 10:
        ui->lab_pwr->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr100.png);}");
        break;
    default:
        break;
    }
}


void StatusBar::show_message(QString str)
{
    qDebug()<<"show_message";
    info_lab_text_list.append(ui->lab_imformation->text());
    ui->lab_imformation->setText(str);
    timer_message->start();
}

void StatusBar::message_reset()
{
    qDebug()<<"message_reset";
    if(info_lab_text_list.count() > 1){
        info_lab_text_list.removeLast();
        ui->lab_imformation->setText(info_lab_text_list.last());
    }
}


void StatusBar::set_wifi_icon(int w)
{
    switch (w) {
    case wifi_sta_custom:
        ui->lab_wifi->show();
        ui->lab_wifi->setPixmap(QPixmap(":/widgetphoto/wifi/wifi3.png").scaled(ui->lab_wifi->size()));
        break;
    case wifi_ap_custom:
    case wifi_ap_camera:
        ui->lab_wifi->show();
        ui->lab_wifi->setPixmap(QPixmap(":/widgetphoto/wifi/wifi_hot.png").scaled(ui->lab_wifi->size()));
        break;
    case wifi_notwork:
        ui->lab_wifi->hide();
    default:
        ui->lab_wifi->hide();
        ui->lab_wifi->setPixmap(QPixmap(":/widgetphoto/wifi/wifi0.png").scaled(ui->lab_wifi->size()));
        break;
    }
}

void StatusBar::fresh_freq(float f)
{
    freq = f;
    ui->lab_freq->setStyleSheet("QLabel {color:green;}");
    timer_miss_sync->start(MISS_SYNC_TIMES * 1000);       //更新同步状态
    ui->lab_freq->setText(QString("%1Hz").arg(QString::number(freq, 'f', 2)) );
}

void StatusBar::fresh_miss_freq()
{
    ui->lab_freq->setStyleSheet("QLabel {color:white;}");
}

void StatusBar::fresh_gps(QString str)
{
    gps_info = str;
    timer_miss_gps->start(MISS_GPS_TIMES * 1000);       //更新gps状态
    ui->lab_temp->show();
}

void StatusBar::fresh_miss_gps()
{
    gps_info.clear();
    ui->lab_temp->hide();
}

void StatusBar::turn_dark(bool f)
{
     battery->get_screen_state(!f);
}


