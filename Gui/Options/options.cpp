#include "options.h"
#include "ui_optionui.h"
#include "ui_apinfo.h"

#include "../Common/common.h"
#include <QPalette>
#include <QThreadPool>
#include <QHeaderView>
#include <QSettings>
#include <QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QProcess>
#include "IO/File/spacecontrol.h"

#define GRADE_1             4
#define GRADE_2_NORMAL      6
#define GRADE_2_WIFI        2
#define GRADE_2_ADVANCED    6

//#define ENABLE_WIFI         //开启wifi功能

Options::Options(QWidget *parent, G_PARA *g_data) : BaseWidget(NULL, parent),
    ui(new Ui::OptionUi),ui_ap(new Ui::ApInfomation)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);

    data = g_data;
    sql_para = *sqlcfg->get_para();
    _datetime = QDateTime::currentDateTime();
    inputStatus = false;
    ui_init();

#ifdef ENABLE_WIFI
    wifi_init();
#else
    ui->tabWidget->widget(1)->setDisabled(true);
    tab1->setFixedHeight(0);
#endif

    refresh();

    this->hide();
}

Options::~Options()
{
    delete ui;
}

void Options::ui_init()
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
    tab3 = new QLabel(tr("存储管理"),ui->tabWidget->tabBar());
    Common::setTab(tab3);

    ui->tabWidget->tabBar()->setTabButton(0,QTabBar::RightSide,tab0);
    ui->tabWidget->tabBar()->setTabButton(1,QTabBar::RightSide,tab1);
    ui->tabWidget->tabBar()->setTabButton(2,QTabBar::RightSide,tab2);
    ui->tabWidget->tabBar()->setTabButton(3,QTabBar::RightSide,tab3);

    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        ui->tabWidget->widget(i)->setStyleSheet("QWidget {background-color:lightGray;}");
    }

    QButtonGroup *group1 = new QButtonGroup();       //必须新建一个组,才使得两组不互斥
    group1->addButton(ui->rbt_CN);
    group1->addButton(ui->rbt_EN);

    QButtonGroup *group2 = new QButtonGroup();       //必须新建一个组,才使得两组不互斥
    group2->addButton(ui->rbt_key_off);
    group2->addButton(ui->rbt_key_on);

    QSettings settings(QSettings::IniFormat,QSettings::SystemScope,"ZDIT","swbox");                             //读取上次保存的设置
    QString str;
    str = settings.value("ap_name").toString();
    if(!str.isEmpty()){
        ui->lineEdit_wifi_hot_name->setText(str);
    }
    str = settings.value("ap_password").toString();
    if(!str.isEmpty()){
        ui->lineEdit_wifi_hot_password->setText(str);
    }
    str = settings.value("ap_gate").toString();
    if(!str.isEmpty()){
        ui->lineEdit_wifi_hot_gate->setText(str);
    }
    str = settings.value("ap_mask").toString();
    if(!str.isEmpty()){
        ui->lineEdit_wifi_hot_mask->setText(str);
    }

    ui->label_mask->hide();
    ui->label_gateway->hide();
    ui->lineEdit_wifi_hot_mask->hide();
    ui->lineEdit_wifi_hot_gate->hide();

    spaceMangagementIni();
}

void Options::wifi_init()
{
    QWidget *w = ui->tabWidget;
    ap_info_widget = new QFrame(w);
    ui_ap->setupUi(ap_info_widget);
    int tab_width = tab1->width() + 8;
    ap_info_widget->resize(w->width()-6-tab_width, w->height()-4);
    ap_info_widget->move(tab_width + 3, 2);
    ap_info_widget->setStyleSheet("QFrame {background-color:darkGray;}");
    ap_info_widget->hide();

    wifiPassword = new WifiPassword;
    current_ap_index = -1;

    socket = new Socket;
    connect(socket,SIGNAL(s_wifi_create_ap(bool)), this, SLOT(wifi_hotpot_finished(bool)));

    wifi = new Wifi;
    thread = new QThread;                       //Options中的附属线程，完成wifi的耗时操作
    wifi->moveToThread(thread);
    connect(thread, SIGNAL(started()), wifi, SLOT(wifi_init()));    //初始化

    connect(this, SIGNAL(ap_fresh_list()), wifi, SLOT(ap_fresh_list()));
    connect(wifi, SIGNAL(ap_fresh_list_complete()), this, SLOT(ap_fresh_list_complete()));          //刷新AP列表完成

    connect(this, SIGNAL(ap_connect(QString,QString)), wifi, SLOT(ap_connect(QString,QString)) );
    connect(wifi, SIGNAL(ap_connect_complete(bool)), this, SLOT(ap_connect_complete(bool)));        //连接成功/失败处理

    connect(this, SIGNAL(ap_refresh_info()), wifi, SLOT(ap_refresh_info()));
    thread->start();
//    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()) );

    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->horizontalHeader()->resizeSection(0,0);
    ui->tableWidget->horizontalHeader()->resizeSection(1,125);
    ui->tableWidget->horizontalHeader()->resizeSection(2,25);

    contextMenu = new QListWidget(this);        //右键菜单
    contextMenu->setStyleSheet("QListWidget {border-image: url(:/widgetphoto/bk/para_child.png);color:gray;outline: none;}");
    QStringList list;
    list << tr("连接/断开连接") << tr("查看连接信息") ;
    contextMenu->addItems(list);
    contextMenu->resize(100, 40);
    contextMenu->move(250,80);
    contextMenu->setSpacing(2);
    contextMenu->hide();

    contextMenu_num = 2;        //显示菜单条目
}

void Options::spaceMangagementIni()
{
    chart = new QChart;
    chart->setTitle(tr("总空间3.2G"));
    chart->setAnimationOptions(QChart::AllAnimations);
    m_series = new QPieSeries();
    *m_series << new QPieSlice(tr("已使用"), 0);
    *m_series << new QPieSlice(tr("未使用"), 100);
    m_series->slices().at(0)->setColor(Qt::blue);
    m_series->slices().at(1)->setColor(Qt::darkGray);
    m_series->setLabelsVisible();
    chart->addSeries(m_series);
    chart->legend()->hide();
    chart->setBackgroundBrush(Qt::lightGray);
    chart->setMargins(QMargins(0,0,0,0));
    ui->chartView->setChart(chart);

    m_series->setPieSize(0.5);

    reloadChart();
}

void Options::reloadChart()
{
    float m_DataLog, m_PRPDLog, m_Asset, m_WaveForm, m_Others;
    m_DataLog = SpaceControl::dirFileSize(DIR_DATALOG);
    m_PRPDLog = SpaceControl::dirFileSize(DIR_PRPDLOG);
    m_Asset = SpaceControl::dirFileSize(DIR_ASSET);
    m_WaveForm = SpaceControl::dirFileSize(DIR_WAVE);
    m_Others = qAbs(SpaceControl::dirFileSize(DIR_DATA) - m_WaveForm - m_Asset - m_PRPDLog - m_DataLog);

    qint64 total, used, available;
    QString persent;
    SpaceControl::disk_info(total, used, available, persent);

    ui->label_data->setText(SpaceControl::size_to_string(m_DataLog));
    ui->label_PRPD->setText(SpaceControl::size_to_string(m_PRPDLog));
    ui->label_Asset->setText(SpaceControl::size_to_string(m_Asset));
    ui->label_Waveform->setText(SpaceControl::size_to_string(m_WaveForm));
    ui->label_Others->setText(SpaceControl::size_to_string(m_Others));

    ui->label_Available->setText(SpaceControl::size_to_string(available * 1000));

    m_series->slices().at(0)->setValue(used);
    m_series->slices().at(1)->setValue(available);
    chart->setTitle(tr("总空间%1").arg(SpaceControl::size_to_string((available + used) * 1000)));
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
    if (key_val == NULL || key_val->grade.val0 != 6 || key_val->grade.val1 != 1) {
        return;
    }

    if(inputStatus){        //文字输入状态
        emit send_input_key(key_code);
        return;
    }

    BaseWidget::trans_key(key_code);
    refresh();
}

void Options::do_key_ok()
{
#ifdef ENABLE_WIFI
    if(ap_info_widget->isVisible()){
        if(ap_info_widget_button == 0){
            ap_info_widget->hide();
        }
        else{
            qDebug()<<"forget!";
            //忘记密码
        }
        return;
    }
#endif

    if(key_val->grade.val3 != 0){
        if(key_val->grade.val2 == 2){               //wifi界面
#ifdef ENABLE_WIFI
            if(key_val->grade.val3 == 1){           //wifi设置
                if(key_val->grade.val4 == 0){       //刷新wifi列表
                    emit show_indicator(true);
                    emit ap_fresh_list();
                }
                else{
                    switch (key_val->grade.val5) {
                    case 0:
                    case 1:         //连接/断开连接
                        wifi_connect();
                        break;
                    case 2:         //显示信息
                        qDebug()<<"show info!!";
                        emit ap_refresh_info();

                        wifi->ap_connect_output();
                        qDebug()<<"current_ap_index:"<<current_ap_index;
                        ap_info_widget_init();
                        ap_info_widget->show();
                        break;
                    default:
                        break;
                    }
                    key_val->grade.val5 = 0;
                }
            }
            else {              //wifi热点
                switch (key_val->grade.val4) {
                case 0:         //开启热点
//                    socket->wifi_create_ap(ui->lineEdit_wifi_hot_name->text(), ui->lineEdit_wifi_hot_password->text());
                    wifi->ap_create(ui->lineEdit_wifi_hot_name->text(),ui->lineEdit_wifi_hot_password->text());
                    emit show_indicator(true);
                    break;
                case 1:         //修改名称
                    emit show_input(ui->lineEdit_wifi_hot_name->text(),tr("请输入wifi名称"));
                    inputStatus = true;
                    break;
                case 2:         //修改密码
                    emit show_input(ui->lineEdit_wifi_hot_password->text(),tr("请输入wifi密码"));
                    inputStatus = true;
                    break;
                case 3:         //修改网关
                    emit show_input(ui->lineEdit_wifi_hot_gate->text(),tr("请输入网关"));
                    inputStatus = true;
                    break;
                case 4:         //修改子网掩码
                    emit show_input(ui->lineEdit_wifi_hot_mask->text(),tr("请输入子网掩码"));
                    inputStatus = true;
                    break;
                default:
                    break;
                }
            }
#endif
        }
        else if(key_val->grade.val2 == 4){          //存储管理
            if(key_val->grade.val3 == 1){           //智能清除
                qDebug()<<"smart del!";
                spaceMangagementIni();
            }
            else if(key_val->grade.val3 == 2){      //全部清除
                qDebug()<<"del all!";
                Common::del_dir(DIR_DATA);
                spaceMangagementIni();
            }
        }
        else{
            saveOptions();      //保存退出状态
        }
    }
}

void Options::do_key_cancel()
{
#ifdef ENABLE_WIFI
    if(ap_info_widget->isVisible()){
        ap_info_widget->hide();
        return;
    }
#endif
    if(key_val->grade.val5!=0){     //退出右键菜单
        key_val->grade.val5=0;
    }
    else if(key_val->grade.val4!=0){     //退出日期编辑模式
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
}

void Options::do_key_up_down(int d)
{
#ifdef ENABLE_WIFI
    if(ap_info_widget->isVisible()){
        return;
    }
#endif

    if(key_val->grade.val3 == 0){   //在一级菜单
        Common::change_index(key_val->grade.val2, d, GRADE_1 , 1);
#ifndef ENABLE_WIFI
        if(key_val->grade.val2 == 2){
            Common::change_index(key_val->grade.val2, d, GRADE_1 , 1);
        }
#endif
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
            break;
        case 2:     //wifi
            if(key_val->grade.val5 == 0){
                switch (key_val->grade.val3) {
                case 1:                         //wifi连接
                    if(ui->tableWidget->rowCount() > 0){
                        Common::change_index(key_val->grade.val4,d,ui->tableWidget->rowCount(),0);
                    }
                    break;
                case 2:                         //wifi热点
                    Common::change_index(key_val->grade.val4,d,5,0);
                    break;
                default:
                    break;
                }
            }
            else{
                Common::change_index(key_val->grade.val5, d, 2 , 1);
            }
            break;
        case 3:     //高级
            Common::change_index(key_val->grade.val3, d, GRADE_2_ADVANCED , 1);
            switch (key_val->grade.val3) {
            case 4:
                sql_para.sync_mode = sync_none;
                break;
            case 5:
                sql_para.sync_mode = SYNC_INTERNAL;
                break;
            case 6:
                sql_para.sync_mode = SYNC_EXTERNAL;
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

void Options::do_key_left_right(int d)
{
#ifdef ENABLE_WIFI
    if(ap_info_widget->isVisible()){
        Common::change_index(ap_info_widget_button,d,1,0);
        return;
    }
#endif

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
            Common::change_index(sql_para.close_time,d,120,0);
            break;
        case 6:         //语言
            Common::change_value(sql_para.language, EN, CN);
            break;
        default:
            break;
        }
        break;
    case 2:         //wifi
        if(key_val->grade.val4 == 0){                                       //wifi设置 / 热点设置切换
            Common::change_index(key_val->grade.val3,d,2,0);
        }
        else if(key_val->grade.val3 == 1 && key_val->grade.val4 > 0){       //右键菜单
            Common::change_index(key_val->grade.val5,d,2,0);
        }
        break;
    case 3:         //高级
        switch (key_val->grade.val3) {
        case 0:         //进入菜单
            key_val->grade.val3 = 1;
            break;
        case 1:         //文件个数
            Common::change_index(sql_para.max_rec_num, 10 * d, 2000, 50);
            break;
        case 2:         //蜂鸣器
            sql_para.buzzer_on = !sql_para.buzzer_on;
            break;
        case 3:         //自动录波间隔
            Common::change_index(sql_para.auto_rec_interval, d, 100, 0);
            break;
        case 5:         //内同步补偿值
            Common::change_index(sql_para.sync_internal_val, d, 359, 0);
            break;
        case 6:         //外同步补偿值
            Common::change_index(sql_para.sync_external_val, d, 359, 0);
            break;
        default:
            break;
        }
        break;
    case 4:     //存储
        Common::change_index(key_val->grade.val3,d,2,0);
        break;
    default:
        break;
    }
}

void Options::saveOptions()
{
    //保存频率
    data->set_send_para (sp_freq_reg, sql_para.freq_val == 50 ? 0 : 1);
    emit fregChanged(sql_para.freq_val);

    //保存时间
    saveDatetime();

    //语言(需要重启)
    bool flag = false;   //重启标志位
    if(sql_para.language != sqlcfg->get_para()->language){
        flag = true;
    }

    bool sync_flag = false;         //同步标志位
    if(sql_para.sync_mode != sqlcfg->get_para()->sync_mode){
        sync_flag = true;
    }

    sqlcfg->sql_save(&sql_para);    //保存

    emit closeTimeChanged(sql_para.close_time );
    emit update_statusBar(tr("【参数设置】已保存！"));

    if(sync_flag){
        emit change_sync_status();
    }

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

    //录波文件上限
    ui->lineEdit_MaxFileNum->setText(QString("%1").arg(sql_para.max_rec_num));

    //蜂鸣器
    if(sql_para.buzzer_on){
        ui->rbt_buzzer_on->setChecked(true);
    }
    else{
        ui->rbt_buzzer_off->setChecked(true);
    }

    //录波间隔
    ui->lineEdit_interval->setText(QString("%1").arg(sql_para.auto_rec_interval));

    //同步模式
    switch (sql_para.sync_mode) {
    case sync_none:
        ui->rbt_sync_none->setChecked(true);
        break;
    case SYNC_INTERNAL:
        ui->rbt_sync_internal->setChecked(true);
        break;
    case SYNC_EXTERNAL:
        ui->rbt_sync_external->setChecked(true);
        break;
    default:
        break;
    }

    //内同步补偿
    ui->lineEdit_sync_internal->setText(QString("%1").arg(sql_para.sync_internal_val));

    //外同步补偿
    ui->lineEdit_sync_external->setText(QString("%1").arg(sql_para.sync_external_val));


    //以下是ui视觉效果
    if(key_val != NULL){
#ifdef ENABLE_WIFI
        if(ap_info_widget_button == 0){         //关闭
            ui_ap->pbt_forget->setStyleSheet("QPushButton {background-color:darkGray;}");
            ui_ap->pbt_close->setStyleSheet("QPushButton {background-color:gray;}");
        }
        else{                                   //忘记网络
            ui_ap->pbt_forget->setStyleSheet("QPushButton {background-color:gray;}");
            ui_ap->pbt_close->setStyleSheet("QPushButton {background-color:darkGray;}");
        }
#endif

        ui->tabWidget->setCurrentIndex(key_val->grade.val2-1);

        tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
        tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
        tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
        tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");

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
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
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
#ifdef ENABLE_WIFI
            ui->pushButton->setStyleSheet("");
            ui->pushButton_hot->setStyleSheet("");
            ui->tableWidget->setCurrentCell(-1,-1);

            switch (key_val->grade.val3) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");

                break;
            case 1:         //wifi设置
                if(key_val->grade.val5 > 0){
                    contextMenu->show();
                    contextMenu->setCurrentRow(key_val->grade.val5-1);
                }
                else{
                    contextMenu->hide();
                }
                if(key_val->grade.val4 == 0){
                    ui->pushButton->setStyleSheet("QPushButton{background-color:darkGray;}");
                }
                else {
                    ui->tableWidget->setCurrentCell(key_val->grade.val4-1,0);
                    if(ui->tableWidget->currentRow() == current_ap_index){
                        contextMenu->item(0)->setText(tr("断开连接"));
                    }
                    else{
                        contextMenu->item(0)->setText(tr("连接"));
                    }
                }
                break;
            case 2:         //wifi热点
                ui->lineEdit_wifi_hot_name->deselect();
                ui->lineEdit_wifi_hot_password->deselect();
                ui->lineEdit_wifi_hot_gate->deselect();
                ui->lineEdit_wifi_hot_mask->deselect();
                switch (key_val->grade.val4) {
                case 0:
                    ui->pushButton_hot->setStyleSheet("QPushButton{background-color:darkGray;}");
                    break;
                case 1:
                    ui->lineEdit_wifi_hot_name->selectAll();
                    break;
                case 2:
                    ui->lineEdit_wifi_hot_password->selectAll();
                    break;
                case 3:
                    ui->lineEdit_wifi_hot_gate->selectAll();
                    break;
                case 4:
                    ui->lineEdit_wifi_hot_mask->selectAll();
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
            break;
#endif
        case 3:     //高级
            ui->lineEdit_MaxFileNum->deselect();
            ui->lineEdit_interval->deselect();
            ui->lineEdit_sync_external->deselect();
            ui->lineEdit_sync_internal->deselect();
            switch (key_val->grade.val3) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            case 1:     //文件个数
                ui->lineEdit_MaxFileNum->selectAll();
                break;
            case 3:     //录波间隔
                ui->lineEdit_interval->selectAll();
                break;
            case 4:
                ui->rbt_sync_none->setChecked(true);
                break;
            case 5:
                ui->rbt_sync_internal->setChecked(true);
                ui->lineEdit_sync_internal->selectAll();
                break;
            case 6:
                ui->rbt_sync_external->setChecked(true);
                ui->lineEdit_sync_external->selectAll();
                break;
            default:
                break;
            }
            break;
        case 4:     //内存管理
            switch (key_val->grade.val3) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                ui->pbt_del_smart->setStyleSheet("");
                ui->pbt_del_all->setStyleSheet("");
                break;
            case 1:     //智能清理
                ui->pbt_del_smart->setStyleSheet("QPushButton {background-color:gray;}");
                ui->pbt_del_all->setStyleSheet("");
                break;
            case 2:     //全部清除
                ui->pbt_del_smart->setStyleSheet("r");
                ui->pbt_del_all->setStyleSheet("QPushButton {background-color:gray;}");
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

/*************************************************************
 *  wifi操作
 * ***********************************************************/
//ap列表回应
void Options::ap_fresh_list_complete()
{
    emit show_indicator(false);
    ui->tableWidget->clear();       //清空内容,释放指针
    ui->tableWidget->setRowCount(wifi->ap_list.count());

    for (int i = 0; i < wifi->ap_list.count(); ++i) {
        AP_INFO ap = wifi->ap_list.at(i);

        QTableWidgetItem *item = new QTableWidgetItem(QString::number(i));
        ui->tableWidget->setItem(i,0,item);         //序号

        item = new QTableWidgetItem(ap.ssid);
        ui->tableWidget->setItem(i,1,item);         //SSID

        if(ap.has_key){
            item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/lock.png"),QString());
        }
        else{
            item = new QTableWidgetItem(QIcon(),QString());
        }
        ui->tableWidget->setItem(i,2,item);         //秘钥(图标)


        if(ap.quality > 75){
            item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/wifi3_b.png"),QString());
        }
        else if(ap.quality > 50){
            item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/wifi2_b.png"),QString());
        }
        else if(ap.quality > 25){
            item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/wifi1_b.png"),QString());
        }
        else{
            item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/wifi0_b.png"),QString());
        }
        ui->tableWidget->setItem(i,3,item);         //强度(图标)
    }
}

//连接请求
void Options::wifi_connect()
{
    //开启新的连接前,将原图标删除
    ui->tableWidget->setItem(current_ap_index,2,new QTableWidgetItem);
    emit show_wifi_icon(WIFI_MODE::WIFI_NONE);
    emit show_indicator(true);          //开启菊花状态

    QString name = ui->tableWidget->item(ui->tableWidget->currentRow(),1)->text();
    //        QString password = "12345678";
    current_ap_index = ui->tableWidget->currentRow();
    //通过密码本查找密码(to be)
    QString password = wifiPassword->lookup_key(name);
    if(password.isEmpty()){     //没找到,开启虚拟键盘
        emit show_input("",tr("请输入wifi密码"));
        inputStatus = true;
    }
    else{
        emit ap_connect(name, password);
    }
}

//连接回应
void Options::ap_connect_complete(bool f)
{
    emit show_indicator(false);
    if(f){
        emit show_wifi_icon(WIFI_MODE::WIFI_STA);
        emit update_statusBar(tr("连接成功!"));
        QTableWidgetItem *item = new QTableWidgetItem(QIcon(":/widgetphoto/wifi/tick.png"),"");
        ui->tableWidget->setItem(current_ap_index,2,item);             //已连接
    }
    else{
        //        wifiPassword->del_key(ui->tableWidget->item(current_ap_index,1)->text());
        emit update_statusBar(tr("密码错误!请重新尝试"));
        wifi_connect();
    }
}

void Options::input_finished(QString str)
{
    if(inputStatus){            //保证不会收到非此界面的键盘输入
        inputStatus = false;
        if(key_val->grade.val2 == 2 && key_val->grade.val3 == 1){       //wifi设置
            QString name = ui->tableWidget->item(ui->tableWidget->currentRow(),1)->text();
            wifiPassword->add_new(name, str);
            //            socket->wifi_connect_route(name,str);
//            wifi->ap_connect(name,str);
            emit ap_connect(name,str);
        }
        else if(key_val->grade.val2 == 2 && key_val->grade.val3 == 2 && !str.isEmpty()){
            QSettings settings(QSettings::IniFormat,QSettings::SystemScope,"ZDIT","swbox");                             //保存设置信息
            if(key_val->grade.val4 == 1){           //名称
                ui->lineEdit_wifi_hot_name->setText(str);
                ui->lineEdit_wifi_hot_name->selectAll();
                settings.setValue("ap_name", str);
            }
            else if(key_val->grade.val4 == 2){      //密码
                ui->lineEdit_wifi_hot_password->setText(str);
                ui->lineEdit_wifi_hot_password->selectAll();
                settings.setValue("ap_password", str);
            }
            else if(key_val->grade.val4 == 3){      //网关
                ui->lineEdit_wifi_hot_gate->setText(str);
                ui->lineEdit_wifi_hot_gate->selectAll();
                settings.setValue("ap_gate", str);
            }
            else if(key_val->grade.val4 == 4){      //掩码
                ui->lineEdit_wifi_hot_mask->setText(str);
                ui->lineEdit_wifi_hot_mask->selectAll();
                settings.setValue("ap_mask", str);
            }
        }
        refresh();
    }
}


//热点回应
void Options::wifi_hotpot_finished(bool f)
{
    emit show_indicator(false);
    if(f){
        emit show_wifi_icon(WIFI_MODE::WIFI_AP);
        emit update_statusBar(tr("创建wifi热点成功!"));

        QTableWidgetItem *item = new QTableWidgetItem("");
        ui->tableWidget->setItem(current_ap_index,2,item);             //已连接
        //        switch (sql_para.wifi_trans_mode) {
        //        case wifi_ftp:
        ////            socket->ftp_start();
        //            break;
        //        case wifi_telnet:
        ////            socket->telnet_start();
        //            break;
        //        case wifi_104:
        ////            socket->_104_start();
        //            break;
        //        default:
        //            break;
        //        }
    }
    else{
        emit update_statusBar(tr("创建wifi热点失败"));
    }
}


void Options::ap_info_widget_init()
{
    int row = ui->tableWidget->currentRow();
    if(row>-1 && row< wifi->ap_list.count()){
        AP_INFO ap = wifi->ap_list.at(row);
        ui_ap->lab_SSID->setText(ap.ssid);          //名称
        ui_ap->lab_mac->setText(ap.mac);            //mac地址
        if(ap.has_key){                             //安全性
            ui_ap->lab_safty->setText(ap.safty);
        }
        else{
            ui_ap->lab_safty->setText(tr("无"));
        }
        ui_ap->lab_freq->setText(ap.freq);          //频率
        ui_ap->lab_signallevel->setText(QString::number(ap.signal_level));    //强度
        ui_ap->lab_bitrate->setText(ap.bitrate);    //比特率

        if(ap.is_connected){                        //是否连接
            ui_ap->lab_connect_status->setText(tr("已连接"));
        }
        else{
            ui_ap->lab_connect_status->setText(tr("未连接"));
        }

        ui_ap->lab_pd_mac->setText(ap.pd_mac);      //仪器mac地址
        ui_ap->lab_IP->setText(ap.ip);              //ip地址
        ui_ap->lab_mask->setText(ap.mask);          //子网掩码
        ui_ap->lab_saveconnect->setText(tr("未保存"));     //未保存
    }
}






