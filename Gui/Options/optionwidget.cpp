#include "optionwidget.h"
#include "ui_optionwidget.h"
#include <QVBoxLayout>
#include "../Common/common.h"

#define TAB_NUM 4

OptionWidget::OptionWidget(G_PARA *g_data,QWidget *parent) :
    BaseWidget(NULL, parent),
    ui(new Ui::OptionWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);

    data = g_data;
    sql_para = *sqlcfg->get_para();

    ui->tabWidget->setStyleSheet("QTabBar::tab {border: 0px solid white; min-width: 0ex;padding: 2px 0px 2px 8px; }"
                                 "QTabBar::tab:selected{ background:#1C1C28; color:#0EC3FD;  }"
                                 "QTabBar::tab:!selected{ background:transparent;   }"
                                 "QTabWidget::pane{border-width:0px;}"
                                 );

    tab0 = new QLabel(tr("基本设置"),ui->tabWidget->tabBar());
    Common::setTab(tab0);
    tab1 = new QLabel(tr("高级设置"),ui->tabWidget->tabBar());
    Common::setTab(tab1);
    tab2 = new QLabel(tr("存储管理"),ui->tabWidget->tabBar());
    Common::setTab(tab2);
    tab3 = new QLabel(tr("网络设置"),ui->tabWidget->tabBar());
    Common::setTab(tab3);

    if(sqlcfg->get_global()->wifi_enable == false){         //禁用wifi功能
        qDebug()<<sqlcfg->get_global()->mac.mid(sqlcfg->get_global()->mac.count()-5,2);
        tab3->resize(70, 0);
    }

    ui->tabWidget->tabBar()->setTabButton(0,QTabBar::RightSide,tab0);
    ui->tabWidget->tabBar()->setTabButton(1,QTabBar::RightSide,tab1);
    ui->tabWidget->tabBar()->setTabButton(2,QTabBar::RightSide,tab2);
    ui->tabWidget->tabBar()->setTabButton(3,QTabBar::RightSide,tab3);

    for (int i = 0; i < ui->tabWidget->count(); ++i) {
//        ui->tabWidget->widget(i)->setStyleSheet("QWidget {background-color:lightGray;}");
        ui->tabWidget->widget(i)->setStyleSheet("QWidget {background:#1C1C28;}");
    }


    option_basic = new OptionBasic(data, &sql_para, ui->tabWidget->widget(0));
    connect(option_basic, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(option_basic, SIGNAL(save()), this, SLOT(save_options()));

    option_expert = new OptionExpert(&sql_para, ui->tabWidget->widget(1));
    connect(option_expert, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(option_expert, SIGNAL(save()), this, SLOT(save_options()));
    connect(option_expert, SIGNAL(fregChanged(float)), this, SIGNAL(fregChanged(float)));           //同步频率
    connect(option_expert, SIGNAL(gpsChanged(QString)), this, SIGNAL(gpsChanged(QString)));         //gps信息
    connect(option_expert, SIGNAL(send_sync(qint64,qint64)), this, SIGNAL(send_sync(qint64,qint64)), Qt::DirectConnection);     //发送同步时间

    option_storage = new OptionStorage(&sql_para, ui->tabWidget->widget(2));
    connect(option_storage, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(option_storage, SIGNAL(save()), this, SLOT(save_options()));

    option_wifi = new OptionWifi(&sql_para, ui->tabWidget->widget(3));
    connect(option_wifi, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(option_wifi, SIGNAL(save()), this, SLOT(save_options()));
    connect(option_wifi, SIGNAL(show_indicator(bool)), this, SIGNAL(show_indicator(bool)));
    connect(option_wifi, SIGNAL(show_input(QString,QString)), this, SIGNAL(show_input(QString,QString)));
    connect(option_wifi, SIGNAL(show_wifi_icon(int)), this, SIGNAL(show_wifi_icon(int)));

    this->hide();
//    initStyle();
    startTimer(1000 * 60 * 5);      //5分钟后关闭wifi
}

OptionWidget::~OptionWidget()
{
    delete ui;
}

void OptionWidget::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    sql_para = *sqlcfg->get_para();         //显示时,需要重置sql

    fresh();
    option_basic->fresh();
    option_expert->fresh();

    this->show();
}

void OptionWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL || key_val->grade.val0 != 6|| key_val->grade.val1 != 1) {
        return;
    }
    if(option_wifi->getInputStatus()){        //文字输入状态
        emit send_input_key(key_code);
        return;
    }
    BaseWidget::trans_key(key_code);
    fresh();
}

void OptionWidget::input_finished(QString str)
{
    option_wifi->input_finished(str);
}

void OptionWidget::do_tab_quit()
{
    key_val->grade.val3 = 0;
    fresh();
}

void OptionWidget::do_key_ok()
{
    if(key_val->grade.val3 == 0){       //代表此时位于最上层操作
        key_val->grade.val3 = 1;

        switch (key_val->grade.val2) {
        case 1:     //基本
            option_basic->work();
            break;
        case 2:     //高级
            option_expert->work();
            break;
        case 3:     //内存管理
            option_storage->work(1);
            break;
        case 4:     //wifi
            option_wifi->work(1);
            break;
        default:
            break;
        }
    }
    else{                               //位于子界面操作,将键盘事件分配下去
        switch (key_val->grade.val2) {
        case 1:     //基本
            option_basic->do_key_ok();
            break;
        case 2:     //高级
            option_expert->do_key_ok();
            break;
        case 3:     //内存管理
            option_storage->do_key_ok();
            break;
        case 4:     //wifi
            option_wifi->do_key_ok();
            break;
        default:
            break;
        }
    }
}

void OptionWidget::do_key_cancel()
{
    if(key_val->grade.val3 == 0){       //代表此时位于最上层操作
        key_val->grade.val2 = 0;        //完全退出
//        cancel_save();
        this->hide();
        emit fresh_parent();
    }
    else{                               //位于子界面操作,将键盘事件分配下去
        switch (key_val->grade.val2) {
        case 1:     //基本
            option_basic->do_key_cancel();
            break;
        case 2:     //高级
            option_expert->do_key_cancel();
            break;
        case 3:     //内存管理
            option_storage->do_key_cancel();
            break;
        case 4:     //wifi
            option_wifi->do_key_cancel();
            break;
        default:
            break;
        }
    }

}

void OptionWidget::do_key_up_down(int d)
{
    if(key_val->grade.val3 == 0){       //代表此时位于最上层操作
        Common::change_index(key_val->grade.val2, d, TAB_NUM , 1);

        if(sqlcfg->get_global()->wifi_enable == false){         //禁用wifi功能
            if(key_val->grade.val2 == 4){
                Common::change_index(key_val->grade.val2, d, TAB_NUM , 1);
            }
        }
    }
    else{                               //位于子界面操作,将键盘事件分配下去
        switch (key_val->grade.val2) {
        case 1:     //基本
            option_basic->do_key_up_down(d);
            break;
        case 2:     //高级
            option_expert->do_key_up_down(d);
            break;
        case 3:     //内存管理
            option_storage->do_key_up_down(d);
            break;
        case 4:     //wifi
            option_wifi->do_key_up_down(d);
            break;
        default:
            break;
        }
    }
}

void OptionWidget::do_key_left_right(int d)
{
    if(key_val->grade.val3 == 0){       //代表此时位于最上层操作
        key_val->grade.val3 = 1;

        switch (key_val->grade.val2) {
        case 1:     //基本
            option_basic->work();
            break;
        case 2:     //高级
            option_expert->work();
            break;
        case 3:     //内存管理
            option_storage->work(d);
            break;
        case 4:     //wifi
            option_wifi->work(d);
            break;
        default:
            break;
        }
    }
    else{                               //位于子界面操作,将键盘事件分配下去
        switch (key_val->grade.val2) {
        case 1:     //基本
            option_basic->do_key_left_right(d);
            break;
        case 2:     //高级
            option_expert->do_key_left_right(d);
            break;
        case 3:     //内存管理
            option_storage->do_key_left_right(d);
            break;
        case 4:     //wifi
            option_wifi->do_key_left_right(d);
            break;
        default:
            break;
        }
    }
}

void OptionWidget::save_options()
{
    //语言(需要重启)
    bool flag = false;   //重启标志位
    if(sql_para.language != sqlcfg->get_para()->language){
        flag = true;
    }

    sqlcfg->sql_save(&sql_para);

    emit update_statusBar(tr("【参数设置】已保存！"));

    if(flag){
        QCoreApplication::quit();
    }
}


//5分钟后如果没有使用wifi，则关闭wifi
void OptionWidget::timerEvent(QTimerEvent *e)
{
    if(!sqlcfg->get_global()->wifi_working){
        system("/root/wifi/close_wifi.sh");
        qDebug()<<"close wifi!";
    }
    killTimer(e->timerId());
}

void OptionWidget::fresh()
{
    ui->tabWidget->setCurrentIndex(key_val->grade.val2-1);

    tab0->setStyleSheet("QLabel{color:#FFFFFF; background:transparent;}");
    tab1->setStyleSheet("QLabel{color:#FFFFFF; background:transparent;}");
    tab2->setStyleSheet("QLabel{color:#FFFFFF; background:transparent;}");
    tab3->setStyleSheet("QLabel{color:#FFFFFF; background:transparent;}");
    if(key_val->grade.val3 == 0){
        switch (key_val->grade.val2) {
        case 1:     //基本
            tab0->setStyleSheet("QLabel{color:#0EC3FD; background:#1C1C28;}");
            break;
        case 2:     //高级
            tab1->setStyleSheet("QLabel{color:#0EC3FD; background:#1C1C28;}");
            break;
        case 3:     //内存管理
            tab2->setStyleSheet("QLabel{color:#0EC3FD; background:#1C1C28;}");
            break;
        case 4:     //wifi
            tab3->setStyleSheet("QLabel{color:#0EC3FD; background:#1C1C28;}");
            break;
        default:
            break;
        }
    }
}
