#include "optionbasic.h"
#include "ui_optionbasic.h"
#include "Gui/Common/common.h"
#include <time.h>

OptionBasic::OptionBasic(G_PARA *g_data, SQL_PARA *sql, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionBasic)
{
    ui->setupUi(this);

    data = g_data;
    sql_para = sql;
    _datetime = QDateTime::currentDateTime();

    row = 0;
    col = 0;

    QButtonGroup *group1 = new QButtonGroup();       //必须新建一个组,才使得两组不互斥
    group1->addButton(ui->rbt_CN);
    group1->addButton(ui->rbt_EN);

    QButtonGroup *group2 = new QButtonGroup();
    group2->addButton(ui->rbt_key_off);
    group2->addButton(ui->rbt_key_on);

    fresh();
}

OptionBasic::~OptionBasic()
{
    delete ui;
}

//进入设置状态的初始化
void OptionBasic::work()
{
    col = 0;
    row = 1;
    _datetime = QDateTime::currentDateTime();
    fresh();
}

void OptionBasic::do_key_ok()
{
    saveDatetime();     //在基本设置中需要提前保存时间
    row = 0;
    col = 0;
    fresh();
    emit save();
    emit quit();
}

void OptionBasic::do_key_cancel()
{
    if(col != 0){           //退出时间日期编辑
        col = 0;
        fresh();
    }
    else{                   //还原对FPGA的修改
        row = 0;
        *sql_para = *sqlcfg->get_para();
        data->set_send_para (sp_backlight_reg, sqlcfg->get_para()->backlight);              //背光需要重设
        data->set_send_para (sp_keyboard_backlight, sqlcfg->get_para()->key_backlight);     //键盘背光需要重设
        fresh();
        emit quit();
    }
}

void OptionBasic::do_key_up_down(int d)
{    
    if(col != 0){
        switch (ui->dateTimeEdit->currentSection()) {
        case QDateTimeEdit::NoSection:
            break;
        case QDateTimeEdit::YearSection:
            _datetime = _datetime.addYears(-d);
            break;
        case QDateTimeEdit::MonthSection:
            _datetime = _datetime.addMonths(-d);
            break;
        case QDateTimeEdit::DaySection:
            _datetime = _datetime.addDays(-d);
            break;
        case QDateTimeEdit::HourSection:
            _datetime = _datetime.addSecs(-d*3600);
            break;
        case QDateTimeEdit::MinuteSection:
            _datetime = _datetime.addSecs(-d*60);
            break;
        case QDateTimeEdit::SecondSection:
            _datetime = _datetime.addSecs(-d);
            break;
        default:
            break;
        }
    }
    else{
        Common::change_index(row,d,6,1);
    }

    fresh();
}

void OptionBasic::do_key_left_right(int d)
{
    switch (row) {
    case 1:         //频率
        Common::change_value(sql_para->freq_val, 50, 60);
        break;
    case 2:         //背光
        ui->slider_backlight->setValue(ui->slider_backlight->value() + d);
        sql_para->backlight = ui->slider_backlight->value () - 1;
        data->set_send_para (sp_backlight_reg, sql_para->backlight);
        break;
    case 3:         //时间
        if(d<0 && ui->dateTimeEdit->currentSectionIndex() == 0){        //按左键退出(多一种选择)
            col = 0;
            break;
        }
        if(col != 0){
            ui->dateTimeEdit->setCurrentSectionIndex(ui->dateTimeEdit->currentSectionIndex() + d);
        }
        else{
            col = 1;
        }
        break;
    case 4:         //键盘
        Common::change_value(sql_para->key_backlight, 0, 1);
        data->set_send_para (sp_keyboard_backlight, sql_para->key_backlight);
        break;
    case 5:         //关机
        Common::change_index(sql_para->close_time,d,120,0);
        break;
    case 6:         //语言
        Common::change_value(sql_para->language, EN, CN);
        break;
    default:
        break;
    }
    fresh();
}

void OptionBasic::saveDatetime()
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

void OptionBasic::fresh()
{
    //频率
    if(sql_para->freq_val == 50){
        ui->rbt_50Hz->setChecked(true);
    }
    else{
        ui->rbt_60Hz->setChecked(true);
    }

    //屏幕背光
    ui->slider_backlight->setValue(sql_para->backlight + 1);

    //系统时间
    ui->dateTimeEdit->setDateTime(_datetime);

    //键盘背光
    if(sql_para->key_backlight == 0){
        ui->rbt_key_off->setChecked(true);
    }
    else{
        ui->rbt_key_on->setChecked(true);
    }

    //自动关机
    ui->lineEdit_CloseTime->setText(QString("%1").arg(sql_para->close_time));

    //语言
    if(sql_para->language == LANGUAGE::CN){
        ui->rbt_CN->setChecked(true);
    }
    else{
        ui->rbt_EN->setChecked(true);
    }

    ui->lab_freq->setFrameShadow(QFrame::Raised);
    ui->lab_backlight->setFrameShadow(QFrame::Raised);
    ui->lab_datetime->setFrameShadow(QFrame::Raised);
    ui->lab_key_back->setFrameShadow(QFrame::Raised);
    ui->lab_closetime->setFrameShadow(QFrame::Raised);
    ui->lab_lang->setFrameShadow(QFrame::Raised);
    ui->lineEdit_CloseTime->deselect();
    ui->dateTimeEdit->setSelectedSection(QDateTimeEdit::NoSection);
    switch (row) {
    case 1:
        ui->lab_freq->setFrameShadow(QFrame::Sunken);
        break;
    case 2:
        ui->lab_backlight->setFrameShadow(QFrame::Sunken);
        break;
    case 3:
        ui->lab_datetime->setFrameShadow(QFrame::Sunken);
        if(col != 0)            //只有进入编辑状态,才有选中
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
}
