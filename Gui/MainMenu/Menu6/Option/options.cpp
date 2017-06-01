#include "options.h"

#include "ui_optionui.h"

#include <QPalette>

Options::Options(QWidget *parent, G_PARA *g_data) : QFrame(parent),ui(new Ui::OptionUi)
{
    key_val = NULL;
    data = g_data;

    this->resize(455, 185);
    this->move(2, 31);
//    this->setStyleSheet("SelfTest {border-image: url(:/widgetphoto/mainmenu/bk2.png);}");
    this->setStyleSheet("Options {background-color:lightGray;}");

    ui->setupUi(this);
    QButtonGroup *group1 = new QButtonGroup();       //必须新建一个组,才使得两组不互斥
    group1->addButton(ui->rbt_CN);
    group1->addButton(ui->rbt_EN);

    optionIni();
}

Options::~Options()
{
    delete ui;
}

void Options::optionIni()
{
    /* get sql para */
    sql_para = sqlcfg->get_para();

    //时间
    _datetime = QDateTime::currentDateTime();
    ui->dateTimeEdit->setDateTime(_datetime);

    //频率
    _freq = sql_para->freq_val;
    updateCheckBox();

    //背光
    _backlight = sql_para->backlight;
    ui->slider_backlight->setValue(_backlight + 1); //保存值为0-7,显示值为1-8

    //自动录波
    ui->checkBox_1->setChecked(sql_para->tev1_sql.auto_rec);
    ui->checkBox_2->setChecked(sql_para->tev2_sql.auto_rec);

    //自动关机时间
    ui->lineEdit_CloseTime->setText(QString("%1").arg(sql_para->close_time));

    //语言
    if(sql_para->language == LANGUAGE::CN)
        ui->rbt_CN->setChecked(true);
    else
        ui->rbt_EN->setChecked(true);


}

void Options::updateCheckBox()
{
    if(_freq == 50){
        ui->rbt_50Hz->setChecked(true);
    }
    else{
        ui->rbt_60Hz->setChecked(true);
    }
}


void Options::saveOptions()
{
    //保存频率
    saveFreq();

    //保存时间
    saveDatetime();

    //保存背光
    _backlight = ui->slider_backlight->value()-1;
    setBacklight();

    //保存自动录波
    saveAutoRec();

    //保存其他设置
    sql_para->close_time = ui->lineEdit_CloseTime->text().toInt();
    emit closeTimeChanged(sql_para->close_time );
    sqlcfg->sql_save(sql_para);

}

void Options::saveFreq()
{
	data->set_send_para (sp_freq, (_freq == 50) ? 0 : 1);
    sql_para->freq_val = _freq;
    sqlcfg->sql_save(sql_para);
    emit fregChanged(_freq);
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

void Options::setBacklight()
{
    sql_para->backlight = _backlight;
    sqlcfg->sql_save(sql_para);

	data->set_send_para (sp_backlight, _backlight);
}

void Options::saveAutoRec()
{
    //注意，这里保存了双通道的自动录波信号
    //rec1=0 rec2=0 tev_auto_rec.rval=0
    //rec1=1 rec2=0 tev_auto_rec.rval=1
    //rec1=0 rec2=1 tev_auto_rec.rval=2
    //rec1=1 rec2=1 tev_auto_rec.rval=3
	data->set_send_para (sp_tev_auto_rec, sql_para->tev1_sql.auto_rec + 2 * sql_para->tev2_sql.auto_rec);
}


void Options::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

void Options::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val1 != 1 || key_val->grade.val2 != 1) {
        return;
    }

    int temp;


    switch (key_code) {
    case KEY_OK:
        if(key_val->grade.val3 != 0){   //保存退出状态
            if(key_val->grade.val3 == 4 && key_val->grade.val5 == 0){       //自动录波
                sql_para->tev1_sql.auto_rec = !sql_para->tev1_sql.auto_rec;
            }
            else if(key_val->grade.val3 == 4 && key_val->grade.val5 == 1){
                sql_para->tev2_sql.auto_rec = !sql_para->tev2_sql.auto_rec;
            }
            saveOptions();
            emit update_statusBar(tr("【参数设置】已保存！"));
        }
        else{                           //进入设置,做一些初始化工作
            optionIni();
        }
        break;
    case KEY_CANCEL:
        if(key_val->grade.val4!=0){     //退出日期编辑模式
            key_val->grade.val4=0;
        }
        else{
            key_val->grade.val2 = 0;    //其他模式下就全退出了
            key_val->grade.val3 = 0;
            this->hide();
            setBacklight();
            fresh_parent();
        }
        break;
    case KEY_UP:
        if(key_val->grade.val4 == 0){
            if(key_val->grade.val3>1){
                key_val->grade.val3--;
            }
            else{
                key_val->grade.val3 = 6 ;
            }
        }
        else{
            switch (ui->dateTimeEdit->currentSection()) {
            case QDateTimeEdit::NoSection:

                break;
            case QDateTimeEdit::YearSection:
                _datetime = _datetime.addYears(-1);
                break;
            case QDateTimeEdit::MonthSection:
                _datetime = _datetime.addMonths(-1);
                break;
            case QDateTimeEdit::DaySection:
                _datetime = _datetime.addDays(-1);
                break;
            case QDateTimeEdit::HourSection:
                _datetime = _datetime.addSecs(-3600);
                break;
            case QDateTimeEdit::MinuteSection:
                _datetime = _datetime.addSecs(-60);
                break;
            case QDateTimeEdit::SecondSection:
                _datetime = _datetime.addSecs(-1);
                break;
            default:
                break;
            }
        }
        break;
    case KEY_DOWN:
        if(key_val->grade.val4 == 0){
            if(key_val->grade.val3<6){
                key_val->grade.val3++;
            }
            else{
                key_val->grade.val3 = 1 ;
            }
        }
        else{
            switch (ui->dateTimeEdit->currentSection()) {
            case QDateTimeEdit::NoSection:

                break;
            case QDateTimeEdit::YearSection:
                _datetime = _datetime.addYears(1);
                printf("year ++");
                break;
            case QDateTimeEdit::MonthSection:
                _datetime = _datetime.addMonths(1);
                break;
            case QDateTimeEdit::DaySection:
                _datetime = _datetime.addDays(1);
                break;
            case QDateTimeEdit::HourSection:
                _datetime = _datetime.addSecs(3600);
                break;
            case QDateTimeEdit::MinuteSection:
                _datetime = _datetime.addSecs(60);
                break;
            case QDateTimeEdit::SecondSection:
                _datetime = _datetime.addSecs(1);
                break;
            default:
                break;
            }
            ui->dateTimeEdit->setDateTime(_datetime);
        }
        break;
    case KEY_LEFT:
        switch (key_val->grade.val3) {
        case 0:

            break;
        case 1:
            if(_freq == 50){
                _freq = 60;
            }
            else{
                _freq = 50;
            }
            break;
        case 2:
            ui->slider_backlight->setValue(ui->slider_backlight->value() - 1);
            data->set_send_para (sp_backlight, ui->slider_backlight->value () - 1);
            break;
        case 3:
            if(key_val->grade.val4 != 0){
                ui->dateTimeEdit->setCurrentSectionIndex(ui->dateTimeEdit->currentSectionIndex()-1);
            }
            else{
                key_val->grade.val4 = 1;
            }
            break;
        case 4:
            if(key_val->grade.val5 == 0){
                key_val->grade.val5 = 1;
            }
            else{
                key_val->grade.val5 = 0;
            }
            break;
        case 5:
            temp = ui->lineEdit_CloseTime->text().toInt();
            if(temp > 0)
                ui->lineEdit_CloseTime->setText(QString("%1").arg(temp - 1));
            break;
        case 6:
            if(sql_para->language == LANGUAGE::EN){
                sql_para->language = LANGUAGE::CN;
                ui->rbt_CN->setChecked(true);
            }
            else{
                sql_para->language = LANGUAGE::EN;
                ui->rbt_EN->setChecked(true);
            }
            break;

        default:
            break;
        }

        break;

    case KEY_RIGHT:
        switch (key_val->grade.val3) {
        case 0:

            break;
        case 1:
            if(_freq == 50){
                _freq = 60;
            }
            else{
                _freq = 50;
            }
            break;
        case 2:
            ui->slider_backlight->setValue(ui->slider_backlight->value() + 1);
            data->set_send_para (sp_backlight, ui->slider_backlight->value () - 1);
            break;
        case 3:
            if(key_val->grade.val4 != 0){
                ui->dateTimeEdit->setCurrentSectionIndex(ui->dateTimeEdit->currentSectionIndex()+1);
            }
            else{
                key_val->grade.val4 = 1;
            }
            break;
        case 4:
            if(key_val->grade.val5 == 0){
                key_val->grade.val5 = 1;
            }
            else{
                key_val->grade.val5 = 0;
            }
            break;

        case 5:
            temp = ui->lineEdit_CloseTime->text().toInt();
            if(temp < 30)
                ui->lineEdit_CloseTime->setText(QString("%1").arg(temp + 1));
            break;

        case 6:
            if(sql_para->language == LANGUAGE::EN){
                sql_para->language = LANGUAGE::CN;
                ui->rbt_CN->setChecked(true);
            }
            else{
                sql_para->language = LANGUAGE::EN;
                ui->rbt_EN->setChecked(true);
            }
            break;

        default:
            break;
        }
        break;
    default:
        break;
    }
    refresh();
}

void Options::refresh()
{
//    printf("\nkey_val->grade.val1 is : %d",key_val->grade.val1);
//    printf("\tkey_val->grade.val2 is : %d",key_val->grade.val2);
//    printf("\tkey_val->grade.val3 is : %d",key_val->grade.val3);

    ui->dateTimeEdit->setDateTime(_datetime);
    updateCheckBox();
//	data->set_send_para (sp_backlight, ui->slider_backlight->value () - 1);
	
    ui->lab_freq->setFrameShadow(QFrame::Raised);
    ui->lab_freq->setStyleSheet("QLabel {color:black;}");
    ui->lab_backlight->setFrameShadow(QFrame::Raised);
    ui->lab_backlight->setStyleSheet("QLabel {color:black;}");
    ui->lab_datetime->setFrameShadow(QFrame::Raised);
    ui->lab_datetime->setStyleSheet("QLabel {color:black;}");
    ui->lab_auot_rec->setFrameShadow(QFrame::Raised);
    ui->lab_auot_rec->setStyleSheet("QLabel {color:black;}");
    ui->lab_closetime->setFrameShadow(QFrame::Raised);
    ui->lab_closetime->setStyleSheet("QLabel {color:black;}");
    ui->lab_lang->setFrameShadow(QFrame::Raised);
    ui->lab_lang->setStyleSheet("QLabel {color:black;}");
    ui->lineEdit_CloseTime->deselect();
    ui->dateTimeEdit->setSelectedSection(QDateTimeEdit::NoSection);
    switch (key_val->grade.val3) {
    case 1:
        ui->lab_freq->setFrameShadow(QFrame::Sunken);
        ui->lab_freq->setStyleSheet("QLabel {color:white;}");
        break;
    case 2:
        ui->lab_backlight->setFrameShadow(QFrame::Sunken);
        ui->lab_backlight->setStyleSheet("QLabel {color:white;}");
        break;
    case 3:
        ui->lab_datetime->setFrameShadow(QFrame::Sunken);
        ui->lab_datetime->setStyleSheet("QLabel {color:white;}");
        if(key_val->grade.val4 != 0)            //只有进入编辑状态,才有选中
            ui->dateTimeEdit->setSelectedSection(ui->dateTimeEdit->currentSection());
        else
            ui->dateTimeEdit->setSelectedSection(QDateTimeEdit::NoSection);
        break;
    case 4:
        ui->lab_auot_rec->setFrameShadow(QFrame::Sunken);
        ui->lab_auot_rec->setStyleSheet("QLabel {color:white;}");
        break;
    case 5:
        ui->lab_closetime->setFrameShadow(QFrame::Sunken);
        ui->lab_closetime->setStyleSheet("QLabel {color:white;}");
        ui->lineEdit_CloseTime->selectAll();
        break;
    case 6:
        ui->lab_lang->setFrameShadow(QFrame::Sunken);
        ui->lab_lang->setStyleSheet("QLabel {color:white;}");
        break;
    default:
        break;
    }

    if(key_val->grade.val3 == 4 ){
        if(key_val->grade.val5 == 0){
            ui->checkBox_1->setStyleSheet("QCheckBox{background-color: gray;}");
            ui->checkBox_2->setStyleSheet("");
        }
        else{
            ui->checkBox_1->setStyleSheet("");
            ui->checkBox_2->setStyleSheet("QCheckBox{background-color: gray;}");
        }
    }
    else{
        ui->checkBox_1->setStyleSheet("");
        ui->checkBox_2->setStyleSheet("");
    }

    ui->checkBox_1->setChecked(sql_para->tev1_sql.auto_rec);
    ui->checkBox_2->setChecked(sql_para->tev2_sql.auto_rec);
}







