#include "optionbasic.h"
#include "ui_optionbasic.h"
#include "Gui/Common/common.h"
#include <time.h>

OptionBasic::OptionBasic(G_PARA *g_data, SQL_PARA *sql, QWidget *parent) :
    TabWidget(sql,parent),
    ui(new Ui::OptionBasic)
{
    ui->setupUi(this);    
    this->resize(TABWIDGET_X,TABWIDGET_Y);

    data = g_data;
    _datetime = QDateTime::currentDateTime();

    QButtonGroup *group1 = new QButtonGroup();       //必须新建一个组,才使得两组不互斥
    group1->addButton(ui->rbt_CN);
    group1->addButton(ui->rbt_EN);

#ifdef NO_CHINESE       //无中文
    ui->rbt_CN->hide();
    ui->rbt_EN->hide();
    ui->lab_lang->hide();
#define ROW_NUM 5
#else
#define ROW_NUM 6
#endif

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
    if(col!=0){
        qDebug()<<"Datetime saved!";
        Common::save_date_time(_datetime);  //在基本设置中需要提前保存时间
    }
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
    switch (col) {
    case 0:         //切换焦点行
        Common::change_index(row,d,ROW_NUM,1);
        break;
    case 1:         //年
        _datetime = _datetime.addYears(-d);
        break;
    case 2:         //月
        _datetime = _datetime.addMonths(-d);
        break;
    case 3:         //日
        _datetime = _datetime.addDays(-d);
        break;
    case 4:         //时
        _datetime = _datetime.addSecs(-d*3600);
        break;
    case 5:         //分
        _datetime = _datetime.addSecs(-d*60);
        break;
    case 6:         //秒
        _datetime = _datetime.addSecs(-d);
        break;
    case 7:         //时区
        Common::change_index(sql_para->time_zone, d, 12, -12);
        break;
    default:
        break;
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
        Common::change_index(col, d, 7, 1);
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
    QString t_z;
    if(sql_para->time_zone > 0){
        t_z = tr("东%1区").arg(qAbs(sql_para->time_zone));
    }
    else if(sql_para->time_zone < 0){
        t_z = tr("西%1区").arg(qAbs(sql_para->time_zone));
    }
    else{
        t_z = "UTC";
    }
    ui->lineEdit_timezone->setText(t_z);

    //键盘背光
    if(sql_para->key_backlight == 0){
//        ui->rbt_key_off->setChecked(true);
        ui->checkBox_key->setChecked(false);
    }
    else{
//        ui->rbt_key_on->setChecked(true);
        ui->checkBox_key->setChecked(true);
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

    //视觉效果
    ui->lab_freq->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_backlight->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_datetime->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_key_back->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_closetime->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_lang->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lineEdit_CloseTime->deselect();
    ui->dateTimeEdit->setSelectedSection(QDateTimeEdit::NoSection);
    switch (row) {
    case 1:
        ui->lab_freq->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 2:
        ui->lab_backlight->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 3:
        ui->lab_datetime->setStyleSheet("QLabel{color:#0EC3FD;}");
        switch (col) {
        case 0:             //无选择
            ui->dateTimeEdit->setSelectedSection(QDateTimeEdit::NoSection);
            ui->lineEdit_timezone->deselect();
            break;
        case 7:             //选择时区
            ui->dateTimeEdit->setSelectedSection(QDateTimeEdit::NoSection);
            ui->lineEdit_timezone->selectAll();
            break;
        default:            //选择时间日期
            ui->dateTimeEdit->setSelectedSection(ui->dateTimeEdit->sectionAt(col-1));
            ui->lineEdit_timezone->deselect();
            break;
        }
        break;
    case 4:
        ui->lab_key_back->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 5:
        ui->lab_closetime->setStyleSheet("QLabel{color:#0EC3FD;}");
        ui->lineEdit_CloseTime->selectAll();
        break;
    case 6:
        ui->lab_lang->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    default:
        break;
    }
}
