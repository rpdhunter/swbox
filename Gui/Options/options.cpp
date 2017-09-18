#include "options.h"
#include "ui_optionui.h"

#include "../Common/common.h"
#include <QPalette>
//#include "IO/Other/wifi.h"
#include <QThreadPool>
//#include "../Common/wifitools.h"

#define GRADE_1             3
#define GRADE_2_NORMAL      6
#define GRADE_2_WIFI        2
#define GRADE_2_ADVANCED    3


Options::Options(QWidget *parent, G_PARA *g_data) : QFrame(parent),ui(new Ui::OptionUi)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);

    key_val = NULL;
    data = g_data;

    optionIni();
//    wifiIni();

    sql_para = *sqlcfg->get_para();
    _datetime = QDateTime::currentDateTime();
    refresh();

    this->hide();
}

Options::~Options()
{
    delete ui;
}

void Options::optionIni()
{
    ui->tabWidget->setStyleSheet("QTabBar::tab {border: 0px solid white; min-width: 0ex;padding: 2px 0px 2px 8px; }"
                                 "QTabBar::tab:selected{ background:lightGray;  }"
                                 "QTabBar::tab:!selected{ background:transparent;   }"
                                 "QTabWidget::pane{border-width:0px;}"
                                 );
    tab0 = new QLabel(tr("基本设置"),ui->tabWidget->tabBar());
    Common::setTab(tab0);
    tab1 = new QLabel(tr("wifi设置"),ui->tabWidget->tabBar());
    Common::setTab(tab1);
    tab2 = new QLabel(tr("高级设置"),ui->tabWidget->tabBar());
    Common::setTab(tab2);

    ui->tabWidget->tabBar()->setTabButton(0,QTabBar::RightSide,tab0);
    ui->tabWidget->tabBar()->setTabButton(1,QTabBar::RightSide,tab1);
    ui->tabWidget->tabBar()->setTabButton(2,QTabBar::RightSide,tab2);

    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        ui->tabWidget->widget(i)->setStyleSheet("QWidget {background-color:lightGray;}");
    }

    QButtonGroup *group1 = new QButtonGroup();       //必须新建一个组,才使得两组不互斥
    group1->addButton(ui->rbt_CN);
    group1->addButton(ui->rbt_EN);

    QButtonGroup *group2 = new QButtonGroup();       //必须新建一个组,才使得两组不互斥
    group2->addButton(ui->rbt_key_off);
    group2->addButton(ui->rbt_key_on);
}

void Options::wifiIni()
{
    tools = new WifiTools(wifi_config);
    connect(tools, SIGNAL(wifi_list(QStringList)), this, SLOT(refresh_wifilist(QStringList)) );
    QThreadPool::globalInstance()->start(tools);
}

void Options::refresh_wifilist(QStringList list)
{
    ui->listWidget->addItems(list);
}

void Options::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    sql_para = *sqlcfg->get_para();
    _datetime = QDateTime::currentDateTime();
    refresh();

    this->show();
}

void Options::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val0 != 5 || key_val->grade.val1 != 1) {
        return;
    }

    switch (key_code) {
    case KEY_OK:
        if(key_val->grade.val3 != 0){   //保存退出状态
            saveOptions();
//            wifiIni();
        }
        break;
    case KEY_CANCEL:
        if(key_val->grade.val4!=0){     //退出日期编辑模式
            key_val->grade.val4=0;
        }
        else if(key_val->grade.val3 != 0){  //退出次级菜单
            key_val->grade.val3 = 0;
        }
        else{
            key_val->grade.val2 = 0;    //其他模式下就全退出了
            this->hide();
            data->set_send_para (sp_backlight_reg, sqlcfg->get_para()->backlight);      //背光需要重设
            data->set_send_para (sp_keyboard_backlight, sqlcfg->get_para()->key_backlight);
            emit fresh_parent();
        }
        break;
    case KEY_UP:
        do_key_up_down(-1);
        break;
    case KEY_DOWN:
        do_key_up_down(1);
        break;
    case KEY_LEFT:
        do_key_left_right(-1);
        break;
    case KEY_RIGHT:
        do_key_left_right(1);
        break;
    default:
        break;
    }
    refresh();
}

void Options::do_key_up_down(int d)
{
    if(key_val->grade.val3 == 0){   //在一级菜单
        Common::change_index(key_val->grade.val2, d, GRADE_1 , 1);
    }
    else{
        switch (key_val->grade.val2) {
        case 1:     //常规
            if(key_val->grade.val4 == 0){
                Common::change_index(key_val->grade.val3, d, GRADE_2_NORMAL , 1);
            }
            else{
                switch (ui->dateTimeEdit->currentSection()) {
                case QDateTimeEdit::NoSection:
                    break;
                case QDateTimeEdit::YearSection:
                    _datetime = _datetime.addYears(d);
                    break;
                case QDateTimeEdit::MonthSection:
                    _datetime = _datetime.addMonths(d);
                    break;
                case QDateTimeEdit::DaySection:
                    _datetime = _datetime.addDays(d);
                    break;
                case QDateTimeEdit::HourSection:
                    _datetime = _datetime.addSecs(d*3600);
                    break;
                case QDateTimeEdit::MinuteSection:
                    _datetime = _datetime.addSecs(d*60);
                    break;
                case QDateTimeEdit::SecondSection:
                    _datetime = _datetime.addSecs(d);
                    break;
                default:
                    break;
                }
            }
            break;
        case 2:     //wifi

            break;
        case 3:     //高级
            Common::change_index(key_val->grade.val3, d, GRADE_2_ADVANCED , 1);
            break;
        default:
            break;
        }
    }
}

void Options::do_key_left_right(int d)
{
    switch (key_val->grade.val2) {
    case 1:     //常规
        switch (key_val->grade.val3) {
        case 0:         //进入菜单
            key_val->grade.val3 = 1;
            break;
        case 1:         //频率
            Common::change_value(sql_para.freq_val, 50, 60);
            break;
        case 2:         //背光
            ui->slider_backlight->setValue(ui->slider_backlight->value() + d);
            sql_para.backlight = ui->slider_backlight->value () - 1;
            data->set_send_para (sp_backlight_reg, sql_para.backlight);
            break;
        case 3:         //时间
            if(d<0 && ui->dateTimeEdit->currentSectionIndex() == 0){        //按左键退出(多一种选择)
                key_val->grade.val4 = 0;
                break;
            }
            if(key_val->grade.val4 != 0){
                ui->dateTimeEdit->setCurrentSectionIndex(ui->dateTimeEdit->currentSectionIndex() + d);
            }
            else{
                key_val->grade.val4 = 1;
            }
            break;
        case 4:         //键盘
            Common::change_value(sql_para.key_backlight, 0, 1);
            data->set_send_para (sp_keyboard_backlight, sql_para.key_backlight);
            break;
        case 5:         //关机
            Common::change_index(sql_para.close_time,d,30,0);
            break;
        case 6:         //语言
            Common::change_value(sql_para.language, EN, CN);
            break;
        default:
            break;
        }
        break;
    case 2:         //wifi

        break;
    case 3:         //高级
        switch (key_val->grade.val3) {
        case 0:         //进入菜单
            key_val->grade.val3 = 1;
            break;
        case 1:         //文件个数
            Common::change_index(sql_para.max_rec_num, 10 * d, 2000, 50);
            break;
        case 2:         //转存SD卡
            sql_para.file_copy_to_SD = !sql_para.file_copy_to_SD;
            break;
        case 3:         //自动录波间隔
            sql_para.auto_rec_interval += d;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void Options::saveOptions()
{
    //保存频率
    data->set_send_para (sp_freq_reg, sql_para.freq_val);
    emit fregChanged(sql_para.freq_val);

    //保存时间
    saveDatetime();

    //语言(需要重启)
    bool flag = true;   //重启标志位
    if(sql_para.language == sqlcfg->get_para()->language){
        flag = false;
    }

    sqlcfg->sql_save(&sql_para);    //保存

    emit closeTimeChanged(sql_para.close_time );
    emit update_statusBar(tr("【参数设置】已保存！"));

    if(flag){
        QCoreApplication::quit();
    }
}

void Options::saveDatetime()
{
    struct tm settime;
    settime.tm_sec = _datetime.time().second();
    settime.tm_min = _datetime.time().minute();
    settime.tm_hour = _datetime.time().hour();
    settime.tm_mday = _datetime.date().day();
    settime.tm_mon = _datetime.date().month() - 1;
    settime.tm_year = _datetime.date().year() - 1900;

    time_t t;
    t = mktime(&settime);
    stime(&t);
    system("hwclock -w");

    qDebug()<<"datetime saved!\n";
}

void Options::refresh()
{
    //频率
    if(sql_para.freq_val == 50){
        ui->rbt_50Hz->setChecked(true);
    }
    else{
        ui->rbt_60Hz->setChecked(true);
    }

    //屏幕背光
    ui->slider_backlight->setValue(sql_para.backlight + 1);

    //系统时间
    ui->dateTimeEdit->setDateTime(_datetime);

    //键盘背光
    if(sql_para.key_backlight == 0){
        ui->rbt_key_off->setChecked(true);
    }
    else{
        ui->rbt_key_on->setChecked(true);
    }

    //自动关机
    ui->lineEdit_CloseTime->setText(QString("%1").arg(sql_para.close_time));

    //语言
    if(sql_para.language == LANGUAGE::CN){
        ui->rbt_CN->setChecked(true);
    }
    else{
        ui->rbt_EN->setChecked(true);
    }

    ui->lineEdit_MaxFileNum->setText(QString("%1").arg(sql_para.max_rec_num));
    if(sql_para.file_copy_to_SD){
        ui->rbt_sd_on->setChecked(true);
    }
    else{
        ui->rbt_sd_off->setChecked(true);
    }
    ui->lineEdit_interval->setText(QString("%1").arg(sql_para.auto_rec_interval));

    //以下是ui视觉效果
    if(key_val != NULL){
        ui->tabWidget->setCurrentIndex(key_val->grade.val2-1);

        tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
        tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
        tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");

        switch (key_val->grade.val2) {
        case 1:             //基本
            ui->lab_freq->setFrameShadow(QFrame::Raised);
            ui->lab_backlight->setFrameShadow(QFrame::Raised);
            ui->lab_datetime->setFrameShadow(QFrame::Raised);
            ui->lab_key_back->setFrameShadow(QFrame::Raised);
            ui->lab_closetime->setFrameShadow(QFrame::Raised);
            ui->lab_lang->setFrameShadow(QFrame::Raised);
            ui->lineEdit_CloseTime->deselect();
            ui->dateTimeEdit->setSelectedSection(QDateTimeEdit::NoSection);
            switch (key_val->grade.val3) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            case 1:
                ui->lab_freq->setFrameShadow(QFrame::Sunken);
                break;
            case 2:
                ui->lab_backlight->setFrameShadow(QFrame::Sunken);
                break;
            case 3:
                ui->lab_datetime->setFrameShadow(QFrame::Sunken);
                if(key_val->grade.val4 != 0)            //只有进入编辑状态,才有选中
                    ui->dateTimeEdit->setSelectedSection(ui->dateTimeEdit->currentSection());
                else
                    ui->dateTimeEdit->setSelectedSection(QDateTimeEdit::NoSection);
                break;
            case 4:
                ui->lab_key_back->setFrameShadow(QFrame::Sunken);
                break;
            case 5:
                ui->lab_closetime->setFrameShadow(QFrame::Sunken);
                ui->lineEdit_CloseTime->selectAll();
                break;
            case 6:
                ui->lab_lang->setFrameShadow(QFrame::Sunken);
                break;
            default:
                break;
            }
            break;
        case 2:   //wifi
            switch (key_val->grade.val3) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            default:
                break;
            }
            break;
        case 3:     //高级
            switch (key_val->grade.val3) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                ui->lineEdit_MaxFileNum->deselect();
                ui->lineEdit_interval->deselect();
                break;
            case 1:     //文件个数
                ui->lineEdit_MaxFileNum->selectAll();
                break;
            case 3:     //录波间隔
                ui->lineEdit_interval->selectAll();
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}







