#include "optionwidget.h"
#include <QVBoxLayout>
#include "../Common/common.h"
#include "ui_optionwidget.h"


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
                                 "QTabBar::tab:selected{ background:lightGray;  }"
                                 "QTabBar::tab:!selected{ background:transparent;   }"
                                 "QTabWidget::pane{border-width:0px;}"
                                 );
    tab0 = new QLabel(tr("基本设置"),ui->tabWidget->tabBar());
    Common::setTab(tab0);
    tab1 = new QLabel(tr("高级设置"),ui->tabWidget->tabBar());
    Common::setTab(tab1);
    tab2 = new QLabel(tr("存储管理"),ui->tabWidget->tabBar());
    Common::setTab(tab2);
    tab3 = new QLabel(tr("wifi设置"),ui->tabWidget->tabBar());
    Common::setTab(tab3);

    ui->tabWidget->tabBar()->setTabButton(0,QTabBar::RightSide,tab0);
    ui->tabWidget->tabBar()->setTabButton(1,QTabBar::RightSide,tab1);
    ui->tabWidget->tabBar()->setTabButton(2,QTabBar::RightSide,tab2);
    ui->tabWidget->tabBar()->setTabButton(3,QTabBar::RightSide,tab3);

    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        ui->tabWidget->widget(i)->setStyleSheet("QWidget {background-color:lightGray;}");
    }


    option_basic = new OptionBasic(data, &sql_para, ui->tabWidget->widget(0));
    option_basic->resize(398,188);
    connect(option_basic, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(option_basic, SIGNAL(save()), this, SLOT(save_options()));

    option_expert = new OptionExpert(&sql_para, ui->tabWidget->widget(1));
    option_expert->resize(398,188);
    connect(option_expert, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(option_expert, SIGNAL(save()), this, SLOT(save_options()));

    option_storage = new OptionStorage(ui->tabWidget->widget(2));
    option_storage->resize(398,188);
    connect(option_storage, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(option_storage, SIGNAL(save()), this, SLOT(save_options()));

    option_wifi = new OptionWifi(ui->tabWidget->widget(3));
    option_wifi->resize(398,188);
    connect(option_wifi, SIGNAL(quit()), this, SLOT(do_tab_quit()));
    connect(option_wifi, SIGNAL(save()), this, SLOT(save_options()));

    this->hide();

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
//    sql_para = *sqlcfg->get_para();

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
    BaseWidget::trans_key(key_code);
    fresh();
}

void OptionWidget::do_tab_quit()
{
    key_val->grade.val3 = 0;
    fresh();
}

void OptionWidget::do_key_ok()
{
    if(key_val->grade.val3 == 0){       //代表此时位于最上层操作
        save_options();         //执行保存设置
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
        Common::change_index(key_val->grade.val2, d, 4 , 1);
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

void OptionWidget::fresh()
{
    ui->tabWidget->setCurrentIndex(key_val->grade.val2-1);

    tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
    tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
    tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
    tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");

    if(key_val->grade.val3 == 0){
        switch (key_val->grade.val2) {
        case 1:     //基本
            tab0->setStyleSheet("QLabel{border: 1px solid darkGray;}");
            break;
        case 2:     //高级
            tab1->setStyleSheet("QLabel{border: 1px solid darkGray;}");
            break;
        case 3:     //内存管理
            tab2->setStyleSheet("QLabel{border: 1px solid darkGray;}");
            break;
        case 4:     //wifi
            tab3->setStyleSheet("QLabel{border: 1px solid darkGray;}");
            break;
        default:
            break;
        }
    }
}
