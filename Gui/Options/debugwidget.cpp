#include "debugwidget.h"
#include "ui_debugwidget.h"
#include "../Common/common.h"

#define TAB_NUM 5

DebugWidget::DebugWidget(G_PARA *g_data, QWidget *parent) :
    BaseWidget(NULL, parent),
    ui(new Ui::DebugWidget)
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
    tab0 = new QLabel(tr("高频通道"),ui->tabWidget->tabBar());
    Common::setTab(tab0);
    tab1 = new QLabel(tr("低频通道"),ui->tabWidget->tabBar());
    Common::setTab(tab1);
    tab2 = new QLabel(tr("通道配置"),ui->tabWidget->tabBar());
    Common::setTab(tab2);
    tab3 = new QLabel(tr("硬件监测"),ui->tabWidget->tabBar());
    Common::setTab(tab3);
    tab4 = new QLabel(tr("专家模式"),ui->tabWidget->tabBar());
    Common::setTab(tab4);

    ui->tabWidget->tabBar()->setTabButton(0,QTabBar::RightSide,tab0);
    ui->tabWidget->tabBar()->setTabButton(1,QTabBar::RightSide,tab1);
    ui->tabWidget->tabBar()->setTabButton(2,QTabBar::RightSide,tab2);
    ui->tabWidget->tabBar()->setTabButton(3,QTabBar::RightSide,tab3);
    ui->tabWidget->tabBar()->setTabButton(4,QTabBar::RightSide,tab4);

    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        ui->tabWidget->widget(i)->setStyleSheet("QWidget {background:#1C1C28;}");
    }

    debug_high = new DebugChannelHigh(&sql_para, ui->tabWidget->widget(0));
    connect(debug_high, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(debug_high, SIGNAL(save()), this, SLOT(save_options()));

    debug_low = new DebugChannelLow(&sql_para, ui->tabWidget->widget(1));
    connect(debug_low, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(debug_low, SIGNAL(save()), this, SLOT(save_options()));

    debug_setting = new DebugChannelSetting(&sql_para, ui->tabWidget->widget(2));
    connect(debug_setting, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(debug_setting, SIGNAL(save()), this, SLOT(save_options()));

    debug_hardware = new DebugHardware(&sql_para, ui->tabWidget->widget(3));
    connect(debug_hardware, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(debug_hardware, SIGNAL(save()), this, SLOT(save_options()));

    debug_expert = new DebugExpert(&sql_para, ui->tabWidget->widget(4));
    connect(debug_expert, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(debug_expert, SIGNAL(save()), this, SLOT(save_options()));

    this->hide();

    iniPasswordUi();
}

DebugWidget::~DebugWidget()
{
    delete ui;
}

void DebugWidget::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    sql_para = *sqlcfg->get_para();

    fresh();
    debug_high->fresh();
    debug_low->fresh();
    debug_setting->fresh();
    debug_hardware->fresh();
    debug_expert->fresh();

    this->show();
}

void DebugWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL || key_val->grade.val0 != 6|| key_val->grade.val1 != 2) {
        return;
    }
//    BaseWidget::trans_key(key_code);
    switch (key_code) {
    case KEY_OK:
        if(pass){
            do_key_ok();
        }
        else{
            //检查密码是否匹配
            if(password == password_set){
                qDebug()<<"password accepted!";
                this->widget_password->hide();
                ui->tabWidget->show();
                pass = true;
                sql_para = *sqlcfg->get_para();     //重新初始化数值显示
            }
            else{
                if(passwordEdit->text().isEmpty()){
                    //初始状态不显示
                }
                else{
                    resetPassword();
                    lab2->setText("password error!");
                    qDebug()<<"password error!";
                }
            }
        }
        break;
    case KEY_CANCEL:
        do_key_cancel();
        break;
    case KEY_UP:
        if(pass){
            do_key_up_down(-1);
        }
        else{
            password.append("U");
            passwordEdit->setText(passwordEdit->text()+"*");
        }
        break;
    case KEY_DOWN:
        if(pass){
            do_key_up_down(1);
        }
        else{
            password.append("D");
            passwordEdit->setText(passwordEdit->text()+"*");
        }
        break;
    case KEY_LEFT:
        if(pass){
            do_key_left_right(-1);
        }
        else{
            password.append("L");
            passwordEdit->setText(passwordEdit->text()+"*");
        }
        break;
    case KEY_RIGHT:
        if(pass){
            do_key_left_right(1);
        }
        else{
            password.append("R");
            passwordEdit->setText(passwordEdit->text()+"*");
        }
        break;
    default:
        break;
    }
    fresh();
}

void DebugWidget::do_tab_quit()
{
    key_val->grade.val3 = 0;
    fresh();
}

void DebugWidget::save_options()
{
    //通道(需要重启)
    bool flag = true;   //重启标志位
    if(sql_para.menu_l1 == sqlcfg->get_para()->menu_l1 && sql_para.menu_l2 == sqlcfg->get_para()->menu_l2
            && sql_para.menu_h1 == sqlcfg->get_para()->menu_h1 && sql_para.menu_h2 == sqlcfg->get_para()->menu_h2
            && sql_para.menu_double == sqlcfg->get_para()->menu_double && sql_para.menu_asset == sqlcfg->get_para()->menu_asset
            && sql_para.aa1_sql.camera == sqlcfg->get_para()->aa1_sql.camera && sql_para.aa2_sql.camera == sqlcfg->get_para()->aa2_sql.camera
            && sql_para.hfct1_sql.mode_recognition == sqlcfg->get_para()->hfct1_sql.mode_recognition
            && sql_para.hfct2_sql.mode_recognition == sqlcfg->get_para()->hfct2_sql.mode_recognition
            && sql_para.multimachine_mode == sqlcfg->get_para()->multimachine_mode){
        flag = false;
    }
    sqlcfg->sql_save(&sql_para);

    //这些设置中，只有这几个量是需要写入FPGA才能生效的
    Common::write_fpga_offset_debug(data);
    emit update_statusBar(tr("【调试模式】已保存！"));

    if(flag){
        QCoreApplication::quit();
    }
}

void DebugWidget::do_key_ok()
{
    if(key_val->grade.val3 == 0){       //代表此时位于最上层操作
        save_options();         //执行保存设置
    }
    else{                               //位于子界面操作,将键盘事件分配下去
        switch (key_val->grade.val2) {
        case 1:     //高频设置
            debug_high->do_key_ok();
            break;
        case 2:     //低频设置
            debug_low->do_key_ok();
            break;
        case 3:     //通道配置
            debug_setting->do_key_ok();
            break;
        case 4:     //硬件监测
            debug_hardware->do_key_ok();
            break;
        case 5:     //专家模式
            debug_expert->do_key_ok();
            break;
        default:
            break;
        }
    }
}

void DebugWidget::do_key_cancel()
{
    if(key_val->grade.val3 == 0){       //代表此时位于最上层操作
        key_val->grade.val2 = 0;        //完全退出
        this->hide();
        resetPassword();
        emit fresh_parent();
    }
    else{                               //位于子界面操作,将键盘事件分配下去
        switch (key_val->grade.val2) {
        case 1:     //高频设置
            debug_high->do_key_cancel();
            break;
        case 2:     //低频设置
            debug_low->do_key_cancel();
            break;
        case 3:     //通道配置
            debug_setting->do_key_cancel();
            break;
        case 4:     //硬件监测
            debug_hardware->do_key_cancel();
            break;
        case 5:     //专家模式
            debug_expert->do_key_cancel();
            break;
        default:
            break;
        }
    }
}

void DebugWidget::do_key_up_down(int d)
{
    if(key_val->grade.val3 == 0){       //代表此时位于最上层操作
        Common::change_index(key_val->grade.val2, d, TAB_NUM , 1);
    }
    else{                               //位于子界面操作,将键盘事件分配下去
        switch (key_val->grade.val2) {
        case 1:     //高频设置
            debug_high->do_key_up_down(d);
            break;
        case 2:     //低频设置
            debug_low->do_key_up_down(d);
            break;
        case 3:     //通道配置
            debug_setting->do_key_up_down(d);
            break;
        case 4:     //硬件监测
            debug_hardware->do_key_up_down(d);
            break;
        case 5:     //专家模式
            debug_expert->do_key_up_down(d);
            break;
        default:
            break;
        }
    }
}

void DebugWidget::do_key_left_right(int d)
{
    if(key_val->grade.val3 == 0){       //代表此时位于最上层操作
        key_val->grade.val3 = 1;

        switch (key_val->grade.val2) {
        case 1:     //高频设置
            debug_high->work();
            break;
        case 2:     //低频设置
            debug_low->work();
            break;
        case 3:     //通道配置
            debug_setting->work();
            break;
        case 4:     //硬件监测
            debug_hardware->work();
            break;
        case 5:     //专家模式
            debug_expert->work();
            break;
        default:
            break;
        }

    }
    else{                               //位于子界面操作,将键盘事件分配下去
        switch (key_val->grade.val2) {
        case 1:     //高频设置
            debug_high->do_key_left_right(d);
            break;
        case 2:     //低频设置
            debug_low->do_key_left_right(d);
            break;
        case 3:     //通道配置
            debug_setting->do_key_left_right(d);
            break;
        case 4:     //硬件监测
            debug_hardware->do_key_left_right(d);
            break;
        case 5:     //专家模式
            debug_expert->do_key_left_right(d);
            break;
        default:
            break;
        }
    }
}

void DebugWidget::fresh()
{
    ui->tabWidget->setCurrentIndex(key_val->grade.val2-1);

    tab0->setStyleSheet("QLabel{color:#FFFFFF; background:transparent;}");
    tab1->setStyleSheet("QLabel{color:#FFFFFF; background:transparent;}");
    tab2->setStyleSheet("QLabel{color:#FFFFFF; background:transparent;}");
    tab3->setStyleSheet("QLabel{color:#FFFFFF; background:transparent;}");
    tab4->setStyleSheet("QLabel{color:#FFFFFF; background:transparent;}");
    if(key_val->grade.val3 == 0){
        switch (key_val->grade.val2) {
        case 1:     //低频
            tab0->setStyleSheet("QLabel{color:#0EC3FD; background:#1C1C28;}");
            break;
        case 2:     //高频
            tab1->setStyleSheet("QLabel{color:#0EC3FD; background:#1C1C28;}");
            break;
        case 3:     //通道
            tab2->setStyleSheet("QLabel{color:#0EC3FD; background:#1C1C28;}");
            break;
        case 4:     //硬件
            tab3->setStyleSheet("QLabel{color:#0EC3FD; background:#1C1C28;}");
            break;
        case 5:     //高级
            tab4->setStyleSheet("QLabel{color:#0EC3FD; background:#1C1C28;}");
            break;
        default:
            break;
        }
    }
}

void DebugWidget::iniPasswordUi()
{
    widget_password = new QWidget(this);
    widget_password->setStyleSheet("QWidget {background-color:lightGray;}");
    widget_password->resize(455, 185);
    widget_password->move(2, 31);

    lab1 = new QLabel(widget_password);
    lab1->setText(tr("请输入管理员密码，解锁调试模式"));
    lab2 = new QLabel(widget_password);
    passwordEdit = new QLineEdit(widget_password);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(lab1);
    layout->addWidget(passwordEdit);
    layout->addWidget(lab2);
    widget_password->setLayout(layout);

    password_set = "UDLR";  //默认密码

    resetPassword();
}

void DebugWidget::resetPassword()
{
    pass = false;
    password.clear();
    passwordEdit->clear();
    lab2->clear();
    this->widget_password->show();
    ui->tabWidget->hide();
}
