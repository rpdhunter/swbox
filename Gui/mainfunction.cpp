#include "mainfunction.h"
#include "IO/SqlCfg/sqlcfg.h"
#include "Common/common.h"
#include <QApplication>
#include <QScreen>
#include <QDateTime>
#include "Function/Asset/assetwidget.h"
#include "IO/File/spacecontrol.h"
#include <QSettings>

MainFunction::MainFunction(G_PARA *_data, QObject *parent) :
    QObject(parent),data(_data)
{
    buzzer = new Buzzer(data);

    timer_dark = new QTimer();
    timer_dark->setSingleShot(true);
    connect(timer_dark, SIGNAL(timeout()), this, SLOT(screen_dark()) );

    timer_sleep = new QTimer();
    timer_sleep->setSingleShot(true);
    connect(timer_sleep, SIGNAL(timeout()), this, SLOT(system_sleep()) );

    timer_reboot = new QTimer();
    timer_reboot->setSingleShot(true);
    connect(timer_reboot, SIGNAL(timeout()), this, SLOT(system_reboot()) );

    timer_printSc = new QTimer();
    timer_printSc->setSingleShot(true);
    connect(timer_printSc, SIGNAL(timeout()), this, SLOT(reset_power_num()) );

    power_num = 0;

    startTimer(1000);
    time_init();            //重置时间

    set_reboot_time();      //开始关机,熄屏等计时
}

void MainFunction::do_print_screen()
{
    power_num++;
    if(power_num == 2){
        printSc();      //截屏
        qDebug()<<"KEY_POWER"<<power_num;
        emit show_message(tr("截屏已保存"));
        power_num = 0;
    }
    timer_printSc->start(1000);
}

void MainFunction::printSc()
{
    //    QPixmap fullScreenPixmap = this->grab(this->rect());                      //老的截屏方式，只能截取指定Wdiget及其子类
    Common::mk_dir(DIR_SCREENSHOTS);
    QPixmap fullScreenPixmap = QGuiApplication::primaryScreen()->grabWindow(0);     //新截屏方式更加完美
    QString name = QString("ScreenShots-%1.png").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz"));
    QString path = QString(DIR_SCREENSHOTS) + "/" + name;
    bool flag = fullScreenPixmap.save(path,"PNG");
    Common::create_hard_link(path, name);       //硬链接到资产
    if(flag)
        qDebug()<<"fullScreen saved!";
    else
        qDebug()<<"fullScreen failed!";
}

void MainFunction::set_asset_dir(QString new_path)
{
    //修改当前ASSET目录
    strcpy(sqlcfg->get_global()->current_dir, new_path.toLocal8Bit().data());
    emit asset_dir_changed();
}

void MainFunction::system_reboot()
{
    qDebug()<<"system will reboot immediately!";
    emit show_message(tr("系统即将关闭"));
    data->set_send_para(sp_backlight_reg,8);

    save();
    SpaceControl::dir_byNum(DIR_ASSET_NORMAL, 500);         //设定存500个资产测量数据

    system("reboot");
}

void MainFunction::system_sleep()
{
    qDebug()<<"system will sleep immediately!";
    data->set_send_para(sp_backlight_reg,8);
    data->set_send_para(sp_keyboard_backlight,0);   //关闭键盘背光
    //    data->set_send_para(sp_rec_on,0);               //关闭录波
    data->set_send_para(sp_sleeping,0);             //关闭复杂功能
}

void MainFunction::screen_dark()
{
    qDebug()<<"screen will darken immediately!";
    data->set_send_para(sp_backlight_reg,0);
    //    data->set_send_para(sp_rec_on,0);               //关闭录波
    data->set_send_para(sp_keyboard_backlight,0);   //关闭键盘背光
    //    battery->get_screen_state(false);
    emit turn_dark(true);           //灭
}

void MainFunction::set_reboot_time()
{
    if(sqlcfg->get_para()->multimachine_mode != multimachine_close)         //多机互联模式,则关闭关系,熄屏系统
        return;

    //重启关机计时器
    int m = sqlcfg->get_para()->close_time;
    if(m != 0){
        timer_reboot->start(m*60 *1000);
    }
    else if(timer_reboot->isActive()){
        timer_reboot->stop();
        qDebug()<<"reboot timer stoped!";
    }

    //重启休眠计时器
    if(!timer_sleep->isActive()){
        qDebug()<<"screen_close_time"<<sqlcfg->get_para()->screen_close_time;
        data->set_send_para(sp_backlight_reg,sqlcfg->get_para()->backlight);            //恢复屏幕亮度
        data->set_send_para(sp_keyboard_backlight,sqlcfg->get_para()->key_backlight);   //恢复键盘背光
        data->set_send_para(sp_sleeping,1);                                             //开启复杂功能
        //        if(sqlcfg->get_para()->tev1_sql.auto_rec == 1 || sqlcfg->get_para()->tev2_sql.auto_rec == 1
        //               || sqlcfg->get_para()->hfct1_sql.auto_rec == 1 || sqlcfg->get_para()->hfct2_sql.auto_rec == 1){
        //             data->set_send_para(sp_rec_on,1);
        //        }
    }
    timer_sleep->start(sqlcfg->get_para()->screen_close_time * 1000);

    //重启亮屏计时器
    if(!timer_dark->isActive()){
        qDebug()<<"screen_dark_time"<<sqlcfg->get_para()->screen_dark_time;
        data->set_send_para(sp_backlight_reg,sqlcfg->get_para()->backlight);            //恢复屏幕亮度
        data->set_send_para(sp_keyboard_backlight,sqlcfg->get_para()->key_backlight);   //恢复键盘背光
        //        if(sqlcfg->get_para()->tev1_sql.auto_rec == 1 || sqlcfg->get_para()->tev2_sql.auto_rec == 1
        //               || sqlcfg->get_para()->hfct1_sql.auto_rec == 1 || sqlcfg->get_para()->hfct2_sql.auto_rec == 1){
        //             data->set_send_para(sp_rec_on,1);
        //        }
    }
    timer_dark->start(sqlcfg->get_para()->screen_dark_time * 1000);
    emit turn_dark(false);      //亮
}

void MainFunction::do_beep(int menu_index, int red_alert)
{
    if(sqlcfg->get_para()->buzzer_on /*&& menu_index == ui->tabWidget->currentIndex() */&& !buzzer->isRunning()){
        if(red_alert == 2){
            buzzer->red();
        }
        else if(red_alert == 1){
            buzzer->yellow();
        }
    }
}

//轮询,检测是否收到测试开始或结束命令
void MainFunction::timerEvent(QTimerEvent *)
{
    int ret = Common::rdb_check_test_start();
    switch (ret) {
    case DP_OPEN:       //开始测试
        set_asset_dir(AssetWidget::normal_asset_dir_init());
        sqlcfg->get_global()->test_mode = true;
        break;
    case DP_CLOSE:      //结束测试
        save();
        sqlcfg->get_global()->test_mode = false;
        break;
    default:
        break;
    }
}

void MainFunction::time_init()
{
    QSettings settings(QSettings::IniFormat,QSettings::SystemScope,"ZDIT","swbox");
    QDateTime last_datetime = settings.value("datetime").toDateTime();
    int interval = last_datetime.daysTo(QDateTime::currentDateTime());       //距离上次关机过了几天
    qDebug()<<"Last used time is: "<<last_datetime.toString()
           << "\tCurrent time from last used time is:"<<interval << "days";
    if(interval < 0){         //当前时间在上次保存时间之前，认为当前时间有误，使用上次保存时间
        qDebug()<<"Current time is:"<<QDateTime::currentDateTime().toString()
               <<"\nCurrent time error! \nReset system time to "<<last_datetime.toString();
        Common::save_date_time(last_datetime);
    }
}

void MainFunction::save()
{
    QSettings settings(QSettings::IniFormat,QSettings::SystemScope,"ZDIT","swbox");         //保存时间
    settings.setValue("datetime", QDateTime::currentDateTime());
    qDebug()<<"save time! "<< QDateTime::currentDateTime();
    emit save_channel();                            //保存通道
}

void MainFunction::reset_power_num()
{
    power_num = 0;
}
