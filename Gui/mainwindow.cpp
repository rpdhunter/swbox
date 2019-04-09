﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>
#include <QQuickItem>
#include <QPushButton>
#include <QPixmap>
#include <QApplication>
#include <QScreen>
#include "IO/File/spacecontrol.h"

#define LOW_POWER_TIMES     20
#define TAB_NUM     7       //通道数量
#define SETTING_NUM 5       //设置条目数


MainWindow::MainWindow(QSplashScreen *sp, QWidget *parent ) :
    QFrame(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry (0, 0, RESOLUTION_X, RESOLUTION_Y);

    QPalette Pal(this->palette());
    Pal.setColor(QPalette::Background, QColor(36,36,36));
    this->setPalette(Pal);

//    sqlcfg->sql_save(sqlcfg->default_config());       //用于程序崩溃时重置数据

    key_val.val = 0;
    data = new G_PARA;
    memset(data, 0, sizeof(G_PARA));
    buzzer = new Buzzer(data);
    power_num = 0;

    sp->showMessage(tr("正在初始化实时数据库..."),Qt::AlignBottom|Qt::AlignLeft);
    init_rdb();

    sp->showMessage(tr("正在初始化按键..."),Qt::AlignBottom|Qt::AlignLeft);
    keydetect = new KeyDetect(this);
    connect(keydetect, SIGNAL(sendkey(quint8)), this, SLOT(trans_key(quint8)));

    sp->showMessage(tr("正在初始化同步..."),Qt::AlignBottom|Qt::AlignLeft);
    syncThread = new SyncThread(this);

    sp->showMessage(tr("正在设置FPGA..."),Qt::AlignBottom|Qt::AlignLeft);
    fifodata = new FifoData(data);

    sp->showMessage(tr("正在初始化通信..."),Qt::AlignBottom|Qt::AlignLeft);
    modbus = new ModbusSync;

    //注册两个自定义类型
    qRegisterMetaType<VectorList>("VectorList");
    qRegisterMetaType<VectorUList>("VectorUList");
    qRegisterMetaType<MODE>("MODE");

    Common::check_base_dir();       //初始化系统文件夹
    sp->showMessage(tr("正在初始化主菜单..."),Qt::AlignBottom|Qt::AlignLeft);

    menu_init();
    qml_init();
    statusbar_init();
    function_init(sp);
    sp->showMessage(tr("正在初始化系统设置..."),Qt::AlignBottom|Qt::AlignLeft);
    options_init();

    box = new QMessageBox(QMessageBox::Warning,tr("关机"),tr("将要关机.\n确定要关机吗?"),
                          QMessageBox::Ok | QMessageBox::Cancel,this);

    Common::messagebox_show_and_init(box);
    box->hide();

    set_asset_dir(AssetWidget::normal_asset_dir_init());        //初始化资产路径
    for (int i = 0; i < mode_list.count(); ++i) {           //寻找有效的初始通道
        if(mode_list.at(i) != Disable){
            key_val.grade.val0 = i;
            ui->tabWidget->setCurrentIndex(i);            
            break;
        }
    }
    fresh_menu_icon();
    fresh_standed_status_message();

    fifodata->start();                                      //开启数据线程
    keydetect->start();
//    modbus->start();
//    syncThread->start();
//    SpaceControl::removeOldFile_smart();

    Common::rdb_dz_init();
    Common::check_restart_file();                       //检查程序是否完成正常启动
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::menu_init()
{
    ui->tabWidget->setStyleSheet("QTabBar::tab {border: 0px solid white; min-width: 0ex;padding: 5px 1px 0px 1px; }"
                                 "QTabBar::tab:selected{ background:rgb(46, 52, 54);  }"
                                 "QTabBar::tab:!selected{ background:transparent;   }"
                                 "QTabWidget::pane{border-width:0px;}"
                                 );
    menu_icon0 = Common::set_mainwindow_lab(new QLabel(this),0,ui->tabWidget);
    menu_icon1 = Common::set_mainwindow_lab(new QLabel(this),1,ui->tabWidget);
    menu_icon2 = Common::set_mainwindow_lab(new QLabel(this),2,ui->tabWidget);
    menu_icon3 = Common::set_mainwindow_lab(new QLabel(this),3,ui->tabWidget);
    menu_icon4 = Common::set_mainwindow_lab(new QLabel(this),4,ui->tabWidget);
    menu_icon5 = Common::set_mainwindow_lab(new QLabel(this),5,ui->tabWidget);
    menu_icon6 = Common::set_mainwindow_lab(new QLabel(this),6,ui->tabWidget);

    //设置每个通道界面的背景，由于使用样式表，会造成子部件背景色混乱，改用调色板设置
    QPalette Pal(this->palette());
    Pal.setColor(QPalette::Background, QColor(46, 52, 54));
    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        ui->tabWidget->widget(i)->setPalette(Pal);
    }
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(fresh_menu_icon()) );
}

void MainWindow::statusbar_init()
{
    battery = new Battery;
    low_power = LOW_POWER_TIMES;

    timer_time = new QTimer();
    timer_time->setInterval(1000);   //1秒1跳
    timer_time->start();

    timer_batt = new QTimer();
    timer_batt->setInterval(10000);   //10秒1跳
    timer_batt->start();

    timer_reboot =  new QTimer();
    timer_reboot->setSingleShot(true);

    timer_sleep =  new QTimer();
    timer_sleep->setSingleShot(true);

    timer_dark = new QTimer();
    timer_dark->setSingleShot(true);

    timer_message = new QTimer();
    timer_message->setSingleShot(true);
    timer_message->setInterval(2000);       //2秒后恢复显示标准信息

    set_reboot_time();

    connect(timer_time, SIGNAL(timeout()), this, SLOT(fresh_status()) );
//    connect(timer_batt, SIGNAL(timeout()), this, SLOT(fresh_batt()) );
    connect(timer_time, SIGNAL(timeout()), this, SLOT(fresh_batt()) );
    connect(timer_reboot, SIGNAL(timeout()), this, SLOT(system_reboot()) );
    connect(timer_sleep, SIGNAL(timeout()), this, SLOT(system_sleep()) );
    connect(timer_dark, SIGNAL(timeout()), this, SLOT(screen_dark()) );
    connect(timer_message, SIGNAL(timeout()), this, SLOT(fresh_standed_status_message()) );

    if(sqlcfg->get_para()->menu_asset == Disable){          //没配置资产，则状态栏资产图标隐藏
        ui->lab_asset->hide();
    }

    freq = 0;
}

void MainWindow::function_init(QSplashScreen *sp)
{
    tev1_widget = NULL;
    tev2_widget = NULL;
    hfct1_widget = NULL;
    hfct2_widget = NULL;
    uhf1_widget = NULL;
    uhf2_widget = NULL;
    double_widget = NULL;
    aa1_widget = NULL;
    aa2_widget = NULL;
    ae1_widget = NULL;
    ae2_widget = NULL;
    asset_widget = NULL;

    sp->showMessage(tr("正在初始化高频通道..."),Qt::AlignBottom|Qt::AlignLeft);
    channel_init((MODE)sqlcfg->get_para()->menu_h1,0);
    channel_init((MODE)sqlcfg->get_para()->menu_h2,1);
    channel_init((MODE)sqlcfg->get_para()->menu_double,2);

    sp->showMessage(tr("正在初始化低频通道..."),Qt::AlignBottom|Qt::AlignLeft);
    channel_init((MODE)sqlcfg->get_para()->menu_l1,3);
    channel_init((MODE)sqlcfg->get_para()->menu_l2,4);

    sp->showMessage(tr("正在初始化资产管理..."),Qt::AlignBottom|Qt::AlignLeft);
    channel_init((MODE)sqlcfg->get_para()->menu_asset,5);
    mode_list.append(Options_Mode);

    if(tev1_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), tev1_widget, SLOT(trans_key(quint8)) );
        connect(tev1_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //录波
        connect(tev1_widget,SIGNAL(startRecWave(MODE,int)),fifodata,SIGNAL(startRecWave(MODE,int)) );
        connect(fifodata,SIGNAL(waveData(VectorList,MODE)),tev1_widget,SLOT(showWaveData(VectorList,MODE)) );
        //重载数据
        connect(ui->tabWidget,SIGNAL(currentChanged(int)), tev1_widget, SLOT(set_current(int)) );
        //菊花
        connect(tev1_widget,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
        //短录波
        connect(fifodata,SIGNAL(short1_update()), tev1_widget,SLOT(read_short_data()));
        //蜂鸣器
        connect(tev1_widget,SIGNAL(beep(int,int)),this,SLOT(do_beep(int,int)));
        //状态信息传递
        connect(tev1_widget,SIGNAL(update_statusBar(QString)),this,SLOT(show_message(QString)));
    }
    if(tev2_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), tev2_widget, SLOT(trans_key(quint8)) );
        connect(tev2_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //录波
        connect(tev2_widget,SIGNAL(startRecWave(MODE,int)),fifodata,SIGNAL(startRecWave(MODE,int)) );
        connect(fifodata,SIGNAL(waveData(VectorList,MODE)),tev2_widget,SLOT(showWaveData(VectorList,MODE)) );
        //重载数据
        connect(ui->tabWidget,SIGNAL(currentChanged(int)), tev2_widget, SLOT(set_current(int)) );
        //菊花
        connect(tev2_widget,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
        //短录波
        connect(fifodata,SIGNAL(short2_update()), tev2_widget,SLOT(read_short_data()));
        //蜂鸣器
        connect(tev2_widget,SIGNAL(beep(int,int)),this,SLOT(do_beep(int,int)));
        //状态信息传递
        connect(tev2_widget,SIGNAL(update_statusBar(QString)),this,SLOT(show_message(QString)));
    }
    if(hfct1_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), hfct1_widget, SLOT(trans_key(quint8)) );
        connect(hfct1_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //录波
        connect(hfct1_widget,SIGNAL(startRecWave(MODE,int)),fifodata,SIGNAL(startRecWave(MODE,int)) );
        connect(fifodata,SIGNAL(waveData(VectorList,MODE)),hfct1_widget,SLOT(showWaveData(VectorList,MODE)) );
        //重载数据
        connect(ui->tabWidget,SIGNAL(currentChanged(int)), hfct1_widget, SLOT(set_current(int)) );
        //菊花
        connect(hfct1_widget,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
        //短录波
        connect(fifodata,SIGNAL(short1_update()), hfct1_widget,SLOT(read_short_data()));
        //蜂鸣器
        connect(hfct1_widget,SIGNAL(beep(int,int)),this,SLOT(do_beep(int,int)));
        //状态信息传递
        connect(hfct1_widget,SIGNAL(update_statusBar(QString)),this,SLOT(show_message(QString)));
    }
    if(hfct2_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), hfct2_widget, SLOT(trans_key(quint8)) );
        connect(hfct2_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //录波
        connect(hfct2_widget,SIGNAL(startRecWave(MODE,int)),fifodata,SIGNAL(startRecWave(MODE,int)) );
        connect(fifodata,SIGNAL(waveData(VectorList,MODE)),hfct2_widget,SLOT(showWaveData(VectorList,MODE)) );
        //重载数据
        connect(ui->tabWidget,SIGNAL(currentChanged(int)), hfct2_widget, SLOT(set_current(int)) );
        //菊花
        connect(hfct2_widget,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
        //短录波
        connect(fifodata,SIGNAL(short2_update()), hfct2_widget,SLOT(read_short_data()));
        //蜂鸣器
        connect(hfct2_widget,SIGNAL(beep(int,int)),this,SLOT(do_beep(int,int)));
        //状态信息传递
        connect(hfct2_widget,SIGNAL(update_statusBar(QString)),this,SLOT(show_message(QString)));
    }
    if(uhf1_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), uhf1_widget, SLOT(trans_key(quint8)) );
        connect(uhf1_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //录波
        connect(uhf1_widget,SIGNAL(startRecWave(MODE,int)),fifodata,SIGNAL(startRecWave(MODE,int)) );
        connect(fifodata,SIGNAL(waveData(VectorList,MODE)),uhf1_widget,SLOT(showWaveData(VectorList,MODE)) );
        //重载数据
        connect(ui->tabWidget,SIGNAL(currentChanged(int)), uhf1_widget, SLOT(set_current(int)) );
        //菊花
        connect(uhf1_widget,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
        //短录波
        connect(fifodata,SIGNAL(short1_update()), uhf1_widget,SLOT(read_short_data()));
        //蜂鸣器
        connect(uhf1_widget,SIGNAL(beep(int,int)),this,SLOT(do_beep(int,int)));
        //状态信息传递
        connect(uhf1_widget,SIGNAL(update_statusBar(QString)),this,SLOT(show_message(QString)));
    }
    if(uhf2_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), uhf2_widget, SLOT(trans_key(quint8)) );
        connect(uhf2_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //录波
        connect(uhf2_widget,SIGNAL(startRecWave(MODE,int)),fifodata,SIGNAL(startRecWave(MODE,int)) );
        connect(fifodata,SIGNAL(waveData(VectorList,MODE)),uhf2_widget,SLOT(showWaveData(VectorList,MODE)) );
        //重载数据
        connect(ui->tabWidget,SIGNAL(currentChanged(int)), uhf2_widget, SLOT(set_current(int)) );
        //菊花
        connect(uhf2_widget,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
        //短录波
        connect(fifodata,SIGNAL(short2_update()), uhf2_widget,SLOT(read_short_data()));
        //蜂鸣器
        connect(uhf2_widget,SIGNAL(beep(int,int)),this,SLOT(do_beep(int,int)));
        //状态信息传递
        connect(uhf2_widget,SIGNAL(update_statusBar(QString)),this,SLOT(show_message(QString)));
    }
    if(double_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), double_widget, SLOT(trans_key(quint8)) );
        connect(double_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //录波
        connect(fifodata,SIGNAL(waveData(VectorList,MODE)),double_widget,SLOT(showWaveData(VectorList,MODE)) );
        //重载数据
        connect(ui->tabWidget,SIGNAL(currentChanged(int)), double_widget, SLOT(reload(int)) );
    }
    if(aa1_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), aa1_widget, SLOT(trans_key(quint8)) );
        connect(aa1_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //录波
        connect(aa1_widget,SIGNAL(startRecWave(MODE,int)),fifodata,SIGNAL(startRecWave(MODE,int)) );
        connect(fifodata,SIGNAL(waveData(VectorList,MODE)),aa1_widget,SLOT(showWaveData(VectorList,MODE)) );
        //重载数据
        connect(ui->tabWidget,SIGNAL(currentChanged(int)), aa1_widget, SLOT(set_current(int)) );
        //菊花
        connect(aa1_widget,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
        //蜂鸣器
        connect(aa1_widget,SIGNAL(beep(int,int)),this,SLOT(do_beep(int,int)));
        //包络线
        connect(fifodata,SIGNAL(envelope1_update(VectorList)), aa1_widget,SLOT(read_envelope_data(VectorList)));
        //状态信息传递
        connect(aa1_widget,SIGNAL(update_statusBar(QString)),this,SLOT(show_message(QString)));
    }
    if(aa2_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), aa2_widget, SLOT(trans_key(quint8)) );
        connect(aa2_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //录波
        connect(aa2_widget,SIGNAL(startRecWave(MODE,int)),fifodata,SIGNAL(startRecWave(MODE,int)) );
        connect(fifodata,SIGNAL(waveData(VectorList,MODE)),aa2_widget,SLOT(showWaveData(VectorList,MODE)) );
        //重载数据
        connect(ui->tabWidget,SIGNAL(currentChanged(int)), aa2_widget, SLOT(set_current(int)) );
        //菊花
        connect(aa2_widget,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
        //蜂鸣器
        connect(aa2_widget,SIGNAL(beep(int,int)),this,SLOT(do_beep(int,int)));
        //包络线
//        connect(fifodata,SIGNAL(envelope2_update()), aa2_widget,SLOT(read_short_data()));
        connect(fifodata,SIGNAL(envelope2_update(VectorList)), aa2_widget,SLOT(read_envelope_data(VectorList)));
        //状态信息传递
        connect(aa2_widget,SIGNAL(update_statusBar(QString)),this,SLOT(show_message(QString)));
    }
    if(ae1_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), ae1_widget, SLOT(trans_key(quint8)) );
        connect(ae1_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //录波
        connect(ae1_widget,SIGNAL(startRecWave(MODE,int)),fifodata,SIGNAL(startRecWave(MODE,int)) );
        connect(fifodata,SIGNAL(waveData(VectorList,MODE)),ae1_widget,SLOT(showWaveData(VectorList,MODE)) );
        //重载数据
        connect(ui->tabWidget,SIGNAL(currentChanged(int)), ae1_widget, SLOT(set_current(int)) );
        //菊花
        connect(ae1_widget,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
        //蜂鸣器
        connect(ae1_widget,SIGNAL(beep(int,int)),this,SLOT(do_beep(int,int)));
        //包络线
//        connect(fifodata,SIGNAL(envelope1_update()), ae1_widget,SLOT(read_short_data()));
        connect(fifodata,SIGNAL(envelope1_update(VectorList)), ae1_widget,SLOT(read_envelope_data(VectorList)));
        //状态信息传递
        connect(ae1_widget,SIGNAL(update_statusBar(QString)),this,SLOT(show_message(QString)));
//        ae1_widget->reload(3);
    }
    if(ae2_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), ae2_widget, SLOT(trans_key(quint8)) );
        connect(ae2_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //录波
        connect(ae2_widget,SIGNAL(startRecWave(MODE,int)),fifodata,SIGNAL(startRecWave(MODE,int)) );
        connect(fifodata,SIGNAL(waveData(VectorList,MODE)),ae2_widget,SLOT(showWaveData(VectorList,MODE)) );
        //重载数据
        connect(ui->tabWidget,SIGNAL(currentChanged(int)), ae2_widget, SLOT(set_current(int)) );
        //菊花
        connect(ae2_widget,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
        //蜂鸣器
        connect(ae2_widget,SIGNAL(beep(int,int)),this,SLOT(do_beep(int,int)));
        //包络线
        connect(fifodata,SIGNAL(envelope2_update(VectorList)), ae2_widget,SLOT(read_envelope_data(VectorList)));
        //状态信息传递
        connect(ae2_widget,SIGNAL(update_statusBar(QString)),this,SLOT(show_message(QString)));
//        ae2_widget->reload(4);
    }
    if(asset_widget != NULL){
        connect(this, SIGNAL(send_key(quint8)), asset_widget, SLOT(trans_key(quint8)) );
        connect(asset_widget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
        //状态栏
        connect(asset_widget,SIGNAL(current_asset_changed(QString,QString)),this,SLOT(set_current_equ(QString,QString)));
        //软键盘
        connect(asset_widget,SIGNAL(show_input(QString,QString)),this,SIGNAL(show_input(QString,QString)));
        connect(asset_widget,SIGNAL(send_input_key(quint8)),this,SIGNAL(send_input_key(quint8)) );
        connect(this, SIGNAL(input_str(QString)), asset_widget, SLOT(input_finished(QString)) );
    }
}

void MainWindow::channel_init(MODE mode, int index)
{
    switch (index) {
    case 0:
        switch (mode) {
        case TEV1:
            tev1_widget = new TEVWidget(data,&key_val,mode,0,ui->H_Channel1);
            break;
        case HFCT1:
            hfct1_widget = new HFCTWidget(data,&key_val,mode,0,ui->H_Channel1);
            break;
        case UHF1:
            uhf1_widget = new UHFWidget(data,&key_val,mode,0,ui->H_Channel1);
            break;
        default:
            mode = Disable;
            menu_icon0->resize(0, 0);
            break;
        }
        break;
    case 1:
        switch (mode) {
        case TEV2:
            tev2_widget = new TEVWidget(data,&key_val,mode,1,ui->H_Channel2);
            break;
        case HFCT2:
            hfct2_widget = new HFCTWidget(data,&key_val,mode,1,ui->H_Channel2);
            break;
        case UHF2:
            uhf2_widget = new UHFWidget(data,&key_val,mode,1,ui->H_Channel2);
            break;
        default:
            mode = Disable;
            menu_icon1->resize(0, 0);
            break;
        }
        break;
    case 2:
        if(mode == Double_Channel){
            double_widget = new FaultLocation(data,&key_val,mode_list,2,ui->Double_Channel);
        }
        else{
            mode = Disable;
            menu_icon2->resize(0, 0);
        }
        break;
    case 3:
        switch (mode) {
        case AA1:
            aa1_widget = new AAWidget(data,&key_val,mode,3,ui->L_Channel1);
            break;
        case AE1:
            ae1_widget = new AEWidget(data,&key_val,mode,3,ui->L_Channel1);
            break;
        default:
            mode = Disable;
            menu_icon3->resize(0, 0);
            break;
        }
        break;
    case 4:
        switch (mode) {
        case AA2:
            aa2_widget = new AAWidget(data,&key_val,mode,4,ui->L_Channel2);
            break;
        case AE2:
            ae2_widget = new AEWidget(data,&key_val,mode,4,ui->L_Channel2);
            break;
        default:
            mode = Disable;
            menu_icon4->resize(0, 0);
            break;
        }
        break;
    case 5:
        if(mode == ASSET){
            asset_widget = new AssetWidget(&key_val,5,ui->Asset);
        }
        else{
            mode = Disable;
            menu_icon5->resize(0, 0);
        }
        break;
    default:
        break;
    }
    mode_list.append(mode);
}

void MainWindow::options_init()
{
    options = new Options(ui->Options,data);
    option_wdiget = new OptionWidget(data,ui->Options);
    debugset = new DebugSet(data,ui->Options);
    systeminfo = new SystemInfo(ui->Options);
    factoryreset = new FactoryReset(ui->Options);
    recwavemanage = new RecWaveManage(ui->Options);

//    connect(this, SIGNAL(send_key(quint8)), options, SLOT(trans_key(quint8)) );
    connect(this, SIGNAL(send_key(quint8)), option_wdiget, SLOT(trans_key(quint8)) );
    connect(this, SIGNAL(send_key(quint8)), debugset, SLOT(trans_key(quint8)) );
    connect(this, SIGNAL(send_key(quint8)), systeminfo, SLOT(trans_key(quint8)) );
    connect(this, SIGNAL(send_key(quint8)), factoryreset, SLOT(trans_key(quint8)) );
    connect(this, SIGNAL(send_key(quint8)), recwavemanage, SLOT(trans_key(quint8)) );
//    connect(options,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
    connect(option_wdiget,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
    connect(debugset,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
    connect(systeminfo,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
    connect(factoryreset,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
    connect(recwavemanage,SIGNAL(fresh_parent()), this, SLOT(fresh_menu_icon()) );
    //显示信息
    connect(options,SIGNAL(update_statusBar(QString)), this, SLOT(show_message(QString)) );
    connect(options, SIGNAL(closeTimeChanged(int)), this, SLOT(set_reboot_time()) );
    connect(options,SIGNAL(fregChanged(int)),this,SLOT(fresh_status()) );
    connect(debugset,SIGNAL(update_statusBar(QString)), this, SLOT(show_message(QString)) );
    connect(syncThread,SIGNAL(update_statusBar(QString)), this, SLOT(show_message(QString)) );
    //播放声音
    connect(recwavemanage,SIGNAL(play_voice(VectorList)),fifodata,SIGNAL(playVoiceData(VectorList)));
    connect(recwavemanage,SIGNAL(stop_play_voice()),fifodata,SIGNAL(stop_play_voice()));
    connect(fifodata,SIGNAL(playVoiceProgress(int,int,bool)),recwavemanage,SLOT(playVoiceProgress(int,int,bool)));
    //键盘
    connect(options,SIGNAL(show_input(QString,QString)),this,SIGNAL(show_input(QString,QString)));
    connect(options,SIGNAL(send_input_key(quint8)),this,SIGNAL(send_input_key(quint8)) );
    connect(this, SIGNAL(input_str(QString)), options, SLOT(input_finished(QString)) );
    //菊花
    connect(options,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
    connect(recwavemanage,SIGNAL(show_indicator(bool)), this, SLOT(show_busy(bool)) );
    //状态栏
    connect(options,SIGNAL(show_wifi_icon(int)), this, SLOT(set_wifi_icon(int)) );
    //同步
    connect(syncThread, SIGNAL(send_sync(qint64,qint64)), fifodata, SIGNAL(send_sync(qint64,qint64)), Qt::DirectConnection);
    connect(options,SIGNAL(change_sync_status()), syncThread, SLOT(change_thread_status()));
    connect(modbus,SIGNAL(do_sync_immediately()),fifodata, SIGNAL(do_sync_immediately()), Qt::DirectConnection);        //外同步
    connect(modbus, SIGNAL(do_sync_freq(short)), this, SLOT(fresh_freq(short)));
    connect(modbus, SIGNAL(send_sync(qint64,qint64)), fifodata, SIGNAL(send_sync(qint64,qint64)), Qt::DirectConnection);
}

void MainWindow::qml_init()
{
    busyIndicator = new QQuickWidget(this);
    busyIndicator->setClearColor(QColor(Qt::transparent));
    busyIndicator->setSource(QUrl(QStringLiteral("qrc:/Busy.qml")));
}

void MainWindow::trans_key(quint8 key_code)
{
    set_reboot_time();          //接到任何按键，重置重启计时器

    if(key_code != KEY_POWER){
        power_num = 0;          //重置截屏计数器
    }

    switch (key_code) {
    case KEY_OK:
        if(box->isVisible()){
            if(box->defaultButton() == box->button(QMessageBox::Ok)){
                data->set_send_para(sp_backlight_reg,8);
                save_channel();
                SpaceControl::dir_byNum(DIR_ASSET_NORMAL, 500);         //设定存500个资产测量数据
                system("reboot");
//                data->set_send_para(sp_reboot,1);
            }
            box->hide();
            return;
        }
        else if(key_val.grade.val0 == TAB_NUM - 1 && key_val.grade.val2 == 0){
            switch (key_val.grade.val1) {
            case 1:
                key_val.grade.val2 = 1;
//                options->working(&key_val);
                option_wdiget->working(&key_val);
                return;
            case 2:
                key_val.grade.val2 = 1;
                debugset->working(&key_val);
                return;
            case 3:
                key_val.grade.val2 = 1;
                recwavemanage->working(&key_val);
                return;
            case 4:
                key_val.grade.val2 = 1;
                systeminfo->working(&key_val);
                return;
            case 5:
                key_val.grade.val2 = 1;
                factoryreset->working(&key_val);
                return;
            default:
                break;
            }
        }
        break;
    case KEY_CANCEL:
        if(box->isVisible()){
            box->hide();
            return;
        }
        else if (key_val.grade.val0 == TAB_NUM - 1 && key_val.grade.val1 > 0 && (key_val.grade.val2 ==0)){       //只针对设置界面
            key_val.grade.val1 = 0;
        }
        break;
    case KEY_SHUTDOWN:
        if(box->isHidden()){
            Common::messagebox_show_and_init(box);
            return;
        }
        break;
    case KEY_POWER:
        power_num++;
        if(power_num == 2){
            printSc();      //截屏
            qDebug()<<"KEY_POWER"<<power_num;
            show_message(tr("截屏已保存"));
            power_num = 0;
        }
        timer_message->start();
        return;
    case KEY_LEFT:
        do_key_left_right(-1);
        break;
    case KEY_RIGHT:
        do_key_left_right(1);
        break;
    case KEY_UP:
        do_key_up_down(-1);
        break;
    case KEY_DOWN:
        do_key_up_down(1);
        break;    
    default:
        break;
    }

    fresh_grade1();
    fresh_menu_icon();
    if((key_val.grade.val0 != TAB_NUM - 1) && box->isHidden()){         //发送给各通道键盘信息
        emit send_key(key_code);
    }
    if(key_val.grade.val0 == TAB_NUM - 1 && key_val.grade.val2 !=0 && box->isHidden()){     //发送到设置界面键盘信息
        emit send_key(key_code);
    }
}

void MainWindow::do_key_up_down(int d)
{
//    qDebug()<<key_val.grade.val0<<"\t"<<key_val.grade.val1<<"\t"<<key_val.grade.val2;
    if (box->isHidden() && (key_val.grade.val0 == TAB_NUM - 1) && (key_val.grade.val2 == 0) ) {
        Common::change_index(key_val.grade.val1,d,SETTING_NUM,1);
        fresh_menu_icon();
        fresh_standed_status_message();     //刷新标准状态栏
    }
}

void MainWindow::do_key_left_right(int d)
{
    if(box->isVisible()){
        Common::messagebox_switch(box);        
    }
    else if(/*key_val.grade.val0 != TAB_NUM - 1 && */key_val.grade.val1 == 0 ){
        do{
            Common::change_index(key_val.grade.val0, d, TAB_NUM - 1, 0);
            fresh_standed_status_message();     //刷新标准状态栏
        }
        while(mode_list.at(key_val.grade.val0) == Disable);
        ui->tabWidget->setCurrentIndex(key_val.grade.val0);
    }
}

void MainWindow::fresh_menu_icon()
{
    power_num = 0;          //重置截屏计数器

    //先根据图标的刷新模式，绘制非当前图标
    if(key_val.grade.val1 == 0){
        set_non_current_menu_icon();
    }
    else{
        set_disable_menu_icon();
    }

    //再刷新当前图标
    switch (key_val.grade.val0) {
    case 0:
        switch (mode_list.at(0)) {
        case TEV1:
            menu_icon0->setPixmap(QPixmap(":/widgetphoto/menu/TEV1_1.png"));
            break;
        case HFCT1:
            menu_icon0->setPixmap(QPixmap(":/widgetphoto/menu/HFCT1_1.png"));
            break;
        case UHF1:
            menu_icon0->setPixmap(QPixmap(":/widgetphoto/menu/UHF1_1.png"));
            break;
        default:
            break;
        }
        break;
    case 1:
        switch (mode_list.at(1)) {
        case TEV2:
            menu_icon1->setPixmap(QPixmap(":/widgetphoto/menu/TEV2_1.png"));
            break;
        case HFCT2:
            menu_icon1->setPixmap(QPixmap(":/widgetphoto/menu/HFCT2_1.png"));
            break;
        case UHF2:
            menu_icon1->setPixmap(QPixmap(":/widgetphoto/menu/UHF2_1.png"));
            break;
        default:
            break;
        }
        break;
    case 2:
        menu_icon2->setPixmap(QPixmap(":/widgetphoto/menu/Double_1.png"));
        break;
    case 3:
        switch (mode_list.at(3)) {
        case AA1:
            menu_icon3->setPixmap(QPixmap(":/widgetphoto/menu/AA_1.png"));
            break;
        case AE1:
            menu_icon3->setPixmap(QPixmap(":/widgetphoto/menu/AE_1.png"));
            break;
        default:
            break;
        }
        break;
    case 4:
        switch (mode_list.at(4)) {
        case AA2:
            menu_icon4->setPixmap(QPixmap(":/widgetphoto/menu/AA_1.png"));
            break;
        case AE2:
            menu_icon4->setPixmap(QPixmap(":/widgetphoto/menu/AE_1.png"));
            break;
        default:
            break;
        }
        break;
    case 5:
        menu_icon5->setPixmap(QPixmap(":/widgetphoto/menu/ASSET_1.png"));
        break;
    case 6:
        menu_icon6->setPixmap(QPixmap(":/widgetphoto/menu/Option_1.png"));
        break;
    default:
        break;
    }
}

void MainWindow::fresh_standed_status_message()
{
    switch (key_val.grade.val0) {
    case 0:
        switch (mode_list.at(0)) {
        case TEV1:
            ui->lab_imformation->setText(tr("地电波检测(高频通道1)"));
            break;
        case HFCT1:
            ui->lab_imformation->setText(tr("电缆局放检测(高频通道1)"));
            break;
        case UHF1:
            ui->lab_imformation->setText(tr("特高频检测(高频通道1)"));
            break;
        default:
            break;
        }
        break;
    case 1:
        switch (mode_list.at(1)) {
        case TEV2:
            ui->lab_imformation->setText(tr("地电波检测(高频通道2)"));
            break;
        case HFCT2:
            ui->lab_imformation->setText(tr("电缆局放检测(高频通道2)"));
            break;
        case UHF2:
            ui->lab_imformation->setText(tr("特高频检测(高频通道2)"));
            break;
        default:
            break;
        }
        break;
    case 2:
        ui->lab_imformation->setText(tr("双通道检测"));
        break;
    case 3:
        switch (mode_list.at(3)) {
        case AA1:
            ui->lab_imformation->setText(tr("AA超声波检测(低频通道1)"));
            break;
        case AE1:
            ui->lab_imformation->setText(tr("AE超声波检测(低频通道1)"));
            break;
        default:
            break;
        }
        break;
    case 4:
        switch (mode_list.at(4)) {
        case AA2:
            ui->lab_imformation->setText(tr("AA超声波检测(低频通道2)"));
            break;
        case AE2:
            ui->lab_imformation->setText(tr("AE超声波检测(低频通道2)"));
            break;
        default:
            break;
        }
        break;
    case 5:
        ui->lab_imformation->setText(tr("资产管理"));
        break;
    case 6:
        ui->lab_imformation->setText(tr("系统设置"));
        if (!key_val.grade.val1) {
            ui->lab_imformation->setText(tr("系统设置"));
        } else if (key_val.grade.val1 == 1){
            ui->lab_imformation->setText(tr("系统设置-参数设置"));
        } else if (key_val.grade.val1 == 2) {
            ui->lab_imformation->setText(tr("系统设置-调试模式"));
        } else if (key_val.grade.val1 == 3){
            ui->lab_imformation->setText(tr("系统设置-录波管理"));
        } else if (key_val.grade.val1 == 4) {
            ui->lab_imformation->setText(tr("系统设置-系统信息"));
        } else if (key_val.grade.val1 == 5) {
            ui->lab_imformation->setText(tr("系统设置-恢复出厂"));
        }
        break;
    default:
        break;
    }
}

//非活动菜单
//存在于光标位于顶层菜单，可用左右键切换时，非当前菜单的图标
//资源文件后缀为2
void MainWindow::set_non_current_menu_icon()
{
    switch (mode_list.at(0)) {
    case TEV1:
        menu_icon0->setPixmap(QPixmap(":/widgetphoto/menu/TEV1_2.png"));
        break;
    case HFCT1:
        menu_icon0->setPixmap(QPixmap(":/widgetphoto/menu/HFCT1_2.png"));
        break;
    case UHF1:
        menu_icon0->setPixmap(QPixmap(":/widgetphoto/menu/UHF1_2.png"));
        break;
    default:                     //禁用
        menu_icon0->setPixmap(QPixmap(":/widgetphoto/menu/TEV1_0.png"));
        break;
    }

    switch (mode_list.at(1)) {
    case TEV2:
        menu_icon1->setPixmap(QPixmap(":/widgetphoto/menu/TEV2_2.png"));
        break;
    case HFCT2:
        menu_icon1->setPixmap(QPixmap(":/widgetphoto/menu/HFCT2_2.png"));
        break;
    case UHF2:
        menu_icon1->setPixmap(QPixmap(":/widgetphoto/menu/UHF2_2.png"));
        break;
    default:                     //禁用
        menu_icon1->setPixmap(QPixmap(":/widgetphoto/menu/TEV2_0.png"));
        break;
    }

    switch (mode_list.at(2)) {
    case Double_Channel:
        menu_icon2->setPixmap(QPixmap(":/widgetphoto/menu/Double_2.png"));
        break;
    default:                     //禁用
        menu_icon2->setPixmap(QPixmap(":/widgetphoto/menu/Double_0.png"));
        break;
    }

    switch (mode_list.at(3)) {
    case AA1:
        menu_icon3->setPixmap(QPixmap(":/widgetphoto/menu/AA_2.png"));
        break;
    case AE1:
        menu_icon3->setPixmap(QPixmap(":/widgetphoto/menu/AE_2.png"));
        break;
    default:                     //禁用
        menu_icon3->setPixmap(QPixmap(":/widgetphoto/menu/AA_0.png"));
        break;
    }

    switch (mode_list.at(4)) {
    case AA2:
        menu_icon4->setPixmap(QPixmap(":/widgetphoto/menu/AA_2.png"));
        break;
    case AE2:
        menu_icon4->setPixmap(QPixmap(":/widgetphoto/menu/AE_2.png"));
        break;
    default:                     //禁用
        menu_icon4->setPixmap(QPixmap(":/widgetphoto/menu/AE_0.png"));
        break;
    }

    switch (mode_list.at(5)) {
    case ASSET:
        menu_icon5->setPixmap(QPixmap(":/widgetphoto/menu/ASSET_2.png"));
        break;
    default:
        menu_icon5->setPixmap(QPixmap(":/widgetphoto/menu/ASSET_0.png"));
        break;
    }

    menu_icon6->setPixmap(QPixmap(":/widgetphoto/menu/Option_2.png"));
}

//禁用菜单
//存在于光标操作子项目，其他全禁用的时候
//资源文件后缀为0
void MainWindow::set_disable_menu_icon()
{
    switch (mode_list.at(0)) {
    case TEV1:
        menu_icon0->setPixmap(QPixmap(":/widgetphoto/menu/TEV1_0.png"));
        break;
    case HFCT1:
        menu_icon0->setPixmap(QPixmap(":/widgetphoto/menu/HFCT1_0.png"));
        break;
    case UHF1:
        menu_icon0->setPixmap(QPixmap(":/widgetphoto/menu/UHF1_0.png"));
        break;
    default:                     //禁用
        menu_icon0->setPixmap(QPixmap(":/widgetphoto/menu/TEV1_0.png"));
        break;
    }

    switch (mode_list.at(1)) {
    case TEV2:
        menu_icon1->setPixmap(QPixmap(":/widgetphoto/menu/TEV2_0.png"));
        break;
    case HFCT2:
        menu_icon1->setPixmap(QPixmap(":/widgetphoto/menu/HFCT2_0.png"));
        break;
    case UHF2:
        menu_icon1->setPixmap(QPixmap(":/widgetphoto/menu/UHF2_0.png"));
        break;
    default:                     //禁用
        menu_icon1->setPixmap(QPixmap(":/widgetphoto/menu/TEV2_0.png"));
        break;
    }

    menu_icon2->setPixmap(QPixmap(":/widgetphoto/menu/Double_0.png"));

    switch (mode_list.at(3)) {
    case AA1:
        menu_icon3->setPixmap(QPixmap(":/widgetphoto/menu/AA_0.png"));
        break;
    case AE1:
        menu_icon3->setPixmap(QPixmap(":/widgetphoto/menu/AE_0.png"));
        break;
    default:                     //禁用
        menu_icon3->setPixmap(QPixmap(":/widgetphoto/menu/AA_0.png"));
        break;
    }

    switch (mode_list.at(4)) {
    case AA2:
        menu_icon4->setPixmap(QPixmap(":/widgetphoto/menu/AA_0.png"));
        break;
    case AE2:
        menu_icon4->setPixmap(QPixmap(":/widgetphoto/menu/AE_0.png"));
        break;
    default:                     //禁用
        menu_icon4->setPixmap(QPixmap(":/widgetphoto/menu/AE_0.png"));
        break;
    }

    menu_icon5->setPixmap(QPixmap(":/widgetphoto/menu/ASSET_0.png"));
    menu_icon6->setPixmap(QPixmap(":/widgetphoto/menu/Option_0.png"));
}

//刷新系统设置页面的二级菜单
void MainWindow::fresh_grade1()
{
    ui->lab_Options->setStyleSheet("QLabel {border-image: url(:/widgetphoto/bk/grade2_bk.png);color:gray;}");
    ui->lab_Debug->setStyleSheet("QLabel {border-image: url(:/widgetphoto/bk/grade2_bk.png);color:gray;}");
    ui->lab_RecWave->setStyleSheet("QLabel {border-image: url(:/widgetphoto/bk/grade2_bk.png);color:gray;}");
    ui->lab_SysInfo->setStyleSheet("QLabel {border-image: url(:/widgetphoto/bk/grade2_bk.png);color:gray;}");
    ui->lab_factory->setStyleSheet("QLabel {border-image: url(:/widgetphoto/bk/grade2_bk.png);color:gray;}");

    switch (key_val.grade.val1) {
    case 1:
        ui->lab_Options->setStyleSheet("QLabel {border-image: url(:/widgetphoto/bk/grade2_bk.png);color:white;}");
        break;
    case 2:
        ui->lab_Debug->setStyleSheet("QLabel {border-image: url(:/widgetphoto/bk/grade2_bk.png);color:white;}");
        break;
    case 3:
        ui->lab_RecWave->setStyleSheet("QLabel {border-image: url(:/widgetphoto/bk/grade2_bk.png);color:white;}");
        break;
    case 4:
        ui->lab_SysInfo->setStyleSheet("QLabel {border-image: url(:/widgetphoto/bk/grade2_bk.png);color:white;}");
        break;
    case 5:
        ui->lab_factory->setStyleSheet("QLabel {border-image: url(:/widgetphoto/bk/grade2_bk.png);color:white;}");
        break;
    default:
        break;
    }
}

//保存各通道数据
//生成测试报告
void MainWindow::save_channel()
{
    if(tev1_widget != NULL){
        tev1_widget->save_channel();
    }
    if(tev2_widget != NULL){
        tev2_widget->save_channel();
    }
    if(hfct1_widget != NULL){
        hfct1_widget->save_channel();
    }
    if(hfct2_widget != NULL){
        hfct2_widget->save_channel();
    }
    if(uhf1_widget != NULL){
        uhf1_widget->save_channel();
    }
    if(uhf2_widget != NULL){
        uhf2_widget->save_channel();
    }
    if(aa1_widget != NULL){
        aa1_widget->save_channel();
    }
    if(aa2_widget != NULL){
        aa2_widget->save_channel();
    }
    if(ae1_widget != NULL){
        ae1_widget->save_channel();
    }
    if(ae2_widget != NULL){
        ae2_widget->save_channel();
    }
}

void MainWindow::set_asset_dir(QString new_path)
{
    //修改当前ASSET目录
    strcpy(sqlcfg->get_global()->current_dir, new_path.toLocal8Bit().data());

//    printf("current dir (2): %s\n", sqlcfg->get_para()->current_dir);

    //通知各界面改变存储路径
    if(tev1_widget != NULL){
        tev1_widget->change_log_dir();
    }
    if(tev2_widget != NULL){
        tev2_widget->change_log_dir();
    }
    if(hfct1_widget != NULL){
        hfct1_widget->change_log_dir();
    }
    if(hfct2_widget != NULL){
        hfct2_widget->change_log_dir();
    }
    if(uhf1_widget != NULL){
        uhf1_widget->change_log_dir();
    }
    if(uhf2_widget != NULL){
        uhf2_widget->change_log_dir();
    }
    if(aa1_widget != NULL){
        aa1_widget->change_log_dir();
    }
    if(aa2_widget != NULL){
        aa2_widget->change_log_dir();
    }
    if(ae1_widget != NULL){
        ae1_widget->change_log_dir();
    }
    if(ae2_widget != NULL){
        ae2_widget->change_log_dir();
    }
}

void MainWindow::set_reboot_time()
{
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
    battery->get_screen_state(true);
}

void MainWindow::fresh_status()
{
    ui->lab_time->setText(QDate::currentDate().toString(tr("yyyy年M月d日"))
                          + " "
                          + QTime::currentTime().toString("h:mm:ss"));

    int s = timer_reboot->remainingTime() / 1000;    //自动关机秒数
    if(timer_reboot->isActive() && s < 60){
        ui->lab_imformation->setText(tr("再过%1秒将自动关机，按任意键取消").arg(s));
    }
    if(freq == 0){
        ui->lab_freq->setStyleSheet("QLabel {color:white;}");
        ui->lab_freq->setText(QString("%1Hz").arg(sqlcfg->get_para()->freq_val));
    }
    else{
        ui->lab_freq->setStyleSheet("QLabel {color:green;}");
        ui->lab_freq->setText(QString("%1Hz").arg(QString("%1Hz").arg(QString::number(freq, 'f', 2))) );
    }
    ui->lab_temp->setText("25°C  40%");
    ui->lab_temp->hide();

    if(Common::rdb_check_test_start()){         //检测测试项目
//        sqlcfg->get_para()
        //保存通道信息
    }
}

void MainWindow::fresh_batt()
{
    //自动关机
    if(battery->is_low_power()){
        low_power--;
        qDebug()<<"low_power = "<<low_power;
        if(low_power == 0){
            system_reboot();
        }
    }
    else{
        low_power = LOW_POWER_TIMES;      //检测错误,重置
    }

    //电量显示
    int batt_val = battery->battPercentValue();


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

void MainWindow::system_reboot()
{
    qDebug()<<"system will reboot immediately!";
    show_message(tr("系统即将关闭"));

    save_channel();

    system("reboot");
}

void MainWindow::system_sleep()
{
    qDebug()<<"system will sleep immediately!";
    data->set_send_para(sp_backlight_reg,8);
    data->set_send_para(sp_keyboard_backlight,0);   //关闭键盘背光
//    data->set_send_para(sp_rec_on,0);               //关闭录波
    data->set_send_para(sp_sleeping,0);             //关闭复杂功能
}

void MainWindow::screen_dark()
{
    qDebug()<<"screen will darken immediately!";
    data->set_send_para(sp_backlight_reg,0);
//    data->set_send_para(sp_rec_on,0);               //关闭录波
    data->set_send_para(sp_keyboard_backlight,0);   //关闭键盘背光
    battery->get_screen_state(false);
}

void MainWindow::show_message(QString str)
{
    ui->lab_imformation->setText(str);
    timer_message->start();
}

void MainWindow::show_busy(bool f)
{
    QObject *busy = busyIndicator->rootObject()->findChild<QObject*>("busy");
    busy->setProperty("running",f);
}

void MainWindow::set_wifi_icon(int w)
{
    switch (w) {
    case WIFI_STA:
        ui->lab_wifi->setPixmap(QPixmap(":/widgetphoto/wifi/wifi3.png").scaled(ui->lab_wifi->size()));
        break;
    case WIFI_AP:
        ui->lab_wifi->setPixmap(QPixmap(":/widgetphoto/wifi/wifi_hot.png").scaled(ui->lab_wifi->size()));
        break;
    case WIFI_SYNC:
        //to be
        break;
    default:
        ui->lab_wifi->setPixmap(QPixmap(":/widgetphoto/wifi/wifi0.png").scaled(ui->lab_wifi->size()));
        break;
    }
}

void MainWindow::do_beep(int menu_index, int red_alert)
{
    if(sqlcfg->get_para()->buzzer_on && menu_index == ui->tabWidget->currentIndex() && !buzzer->isRunning()){
        if(red_alert == 2){
            buzzer->red();
        }
        else if(red_alert == 1){
            buzzer->yellow();
        }
    }
}

void MainWindow::set_current_equ(QString new_equ, QString new_path)
{
    //显示
    ui->lab_asset->setText(new_equ);
    if(new_equ == tr("未指定")){
        ui->lab_asset->setStyleSheet("QLabel {color:white;}");
    }
    else{
        ui->lab_asset->setStyleSheet("QLabel {color:green;}");
    }

    //转换资产目录前的保存工作(AA没有PRPD，不需要保存)
    save_channel();

//    printf("current dir (1): %s\n", new_path.toLocal8Bit().data());

    //设置新的资产路径
    set_asset_dir(new_path);

}

void MainWindow::fresh_freq(short f)
{
    freq = f/100.0;
}

void MainWindow::printSc()
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
