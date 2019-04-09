#include "debugset.h"
#include "ui_debugui.h"
#include <QButtonGroup>
#include <QDir>
#include "../Common/common.h"
#include "IO/Com/rdb/rdb.h"

#define SETTING_NUM             4           //设置菜单条目数
#define SETTING_NUM_HIGH        6
#define SETTING_NUM_LOW         6
#define SETTING_NUM_CHANNEL     6

DebugSet::DebugSet(G_PARA *g_data,QWidget *parent) : BaseWidget(NULL, parent),ui(new Ui::DebugUi)
{
    key_val = NULL;
    data = g_data;
    sql_para = *sqlcfg->get_para();

    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    ui->setupUi(this);

//    battery = new Battery;
    timer_rdb = new QTimer;
    timer_rdb->start(1000);
    connect(timer_rdb, SIGNAL(timeout()), this, SLOT(fresh_rdb_data()) );

    iniUi();
    reload();
    iniPasswordUi();            //密码部件
    ui->tabWidget->setCurrentIndex(0);
    this->hide();
}

void DebugSet::iniUi()
{
    ui->tabWidget->setStyleSheet("QTabBar::tab {border: 0px solid white; min-width: 0ex;padding: 2px 0px 2px 8px; }"
                                 "QTabBar::tab:selected{ background:lightGray;  }"
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

    ui->tabWidget->tabBar()->setTabButton(0,QTabBar::LeftSide,tab0);
    ui->tabWidget->tabBar()->setTabButton(1,QTabBar::LeftSide,tab1);
    ui->tabWidget->tabBar()->setTabButton(2,QTabBar::LeftSide,tab2);
    ui->tabWidget->tabBar()->setTabButton(3,QTabBar::LeftSide,tab3);

    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        ui->tabWidget->widget(i)->setStyleSheet("QWidget {background-color:lightGray;}");
    }

    if(sqlcfg->get_para()->menu_h1 == HFCT1){       //HFCT模式隐藏噪声设置
        ui->lab_noise_h1->hide();
        ui->lineEdit_H1_NOISE->hide();
        ui->lab_offset_noise_h1->hide();
    }
    else{                                           //非HFCT模式隐藏模式判断
        ui->lab_bp_h1->hide();
        ui->rbt_bp_on_h1->hide();
        ui->rbt_bp_off_h1->hide();
    }

    if(sqlcfg->get_para()->menu_h2 == HFCT2){       //HFCT模式隐藏噪声设置
        ui->lab_noise_h2->hide();
        ui->lineEdit_H2_NOISE->hide();
        ui->lab_offset_noise_h2->hide();
    }
    else{                                           //非HFCT模式隐藏模式判断
        ui->lab_bp_h2->hide();
        ui->rbt_bp_on_h2->hide();
        ui->rbt_bp_off_h2->hide();
    }

    if(sqlcfg->get_para()->menu_l1 == AA1){         //AA模式隐藏传感器频率
        ui->label_L1_Freq->hide();
        ui->lineEdit_L1_Freq->hide();
    }
    else{                                           //AE模式隐藏摄像头
        ui->label_L1_Camera->hide();
        ui->rbt_L1_camera_on->hide();
        ui->rbt_L1_camera_off->hide();
    }
    if(sqlcfg->get_para()->menu_l2 == AE2){         //AE模式隐藏摄像头
        ui->label_L2_Camera->hide();
        ui->rbt_L2_camera_on->hide();
        ui->rbt_L2_camera_off->hide();
    }
    else{                                           //AA模式隐藏传感器频率
        ui->label_L2_Freq->hide();
        ui->lineEdit_L2_Freq->hide();
    }

    QString style = "QLabel {font-family:WenQuanYi Micro Hei;font: bold; font-size:16px;color:green}";
    if(sqlcfg->get_para()->menu_h1 == TEV1 || sqlcfg->get_para()->menu_h1 == HFCT1 || sqlcfg->get_para()->menu_h1 == UHF1){
        ui->label_H1->setStyleSheet(style);
        ui->label_H1->setText(Common::mode_to_string((MODE)sqlcfg->get_para()->menu_h1));
    }
    if(sqlcfg->get_para()->menu_h2 == TEV2 || sqlcfg->get_para()->menu_h2 == HFCT2 || sqlcfg->get_para()->menu_h2 == UHF2){
        ui->label_H2->setStyleSheet(style);
        ui->label_H2->setText(Common::mode_to_string((MODE)sqlcfg->get_para()->menu_h2));

    }

    if(sqlcfg->get_para()->menu_l1 == AA1 || sqlcfg->get_para()->menu_l1 == AE1){
        ui->label_L1->setStyleSheet(style);
        ui->label_L1->setText(Common::mode_to_string((MODE)sqlcfg->get_para()->menu_l1));

    }
    if(sqlcfg->get_para()->menu_l2 == AA2 || sqlcfg->get_para()->menu_l2 == AE2){
        ui->label_L2->setStyleSheet(style);
        ui->label_L2->setText(Common::mode_to_string((MODE)sqlcfg->get_para()->menu_l2));
    }

    ui->lineEdit_CompileTime->setText(QString("%1 %2").arg(__TIME__).arg(__DATE__));

    QButtonGroup *group1 = new QButtonGroup(this);
    QButtonGroup *group2 = new QButtonGroup(this);
    QButtonGroup *group3 = new QButtonGroup(this);
    QButtonGroup *group4 = new QButtonGroup(this);
    QButtonGroup *group5 = new QButtonGroup(this);
    QButtonGroup *group6 = new QButtonGroup(this);
    QButtonGroup *group7 = new QButtonGroup(this);
    QButtonGroup *group8 = new QButtonGroup(this);
    QButtonGroup *group9 = new QButtonGroup(this);
    QButtonGroup *group10 = new QButtonGroup(this);
    group1->addButton( ui->rb_HC1_TEV );
    group1->addButton( ui->rb_HC1_HFCT );;
    group1->addButton( ui->rb_HC1_UHF );
    group1->addButton( ui->rb_HC1_Disable );
    group2->addButton( ui->rb_HC2_TEV );
    group2->addButton( ui->rb_HC2_HFCT );
    group2->addButton( ui->rb_HC2_UHF );
    group2->addButton( ui->rb_HC2_Disable );
    group3->addButton( ui->rb_LC1_AA );
    group3->addButton( ui->rb_LC1_AE );
    group3->addButton( ui->rb_LC1_Disable );
    group4->addButton( ui->rb_LC2_AA );
    group4->addButton( ui->rb_LC2_AE );
    group4->addButton( ui->rb_LC2_Disable );
    group5->addButton( ui->rb_Double_Enable );
    group5->addButton( ui->rb_Double_Disable );
    group6->addButton( ui->rbt_L1_envelope );
    group6->addButton( ui->rbt_L1_original );
    group7->addButton( ui->rbt_L2_envelope );
    group7->addButton( ui->rbt_L2_original );
    group8->addButton( ui->rbt_bp_off_h1 );
    group8->addButton( ui->rbt_bp_on_h1 );
    group9->addButton( ui->rbt_bp_off_h2 );
    group9->addButton( ui->rbt_bp_on_h2 );
    group10->addButton( ui->rbt_L1_camera_off );
    group10->addButton( ui->rbt_L1_camera_on );

    chk_status_h1 = 00;
    chk_status_h2 = 00;
}

void DebugSet::iniPasswordUi()
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

void DebugSet::resetPassword()
{
    pass = false;
    password.clear();
    passwordEdit->clear();
    lab2->clear();
    this->widget_password->show();
    ui->tabWidget->hide();
}

void DebugSet::saveSql()
{
    bool flag = true;      //判断是否需要重启标志
    if(sql_para.menu_l1 == sqlcfg->get_para()->menu_l1 && sql_para.menu_l2 == sqlcfg->get_para()->menu_l2
            && sql_para.menu_h1 == sqlcfg->get_para()->menu_h1 && sql_para.menu_h2 == sqlcfg->get_para()->menu_h2
            && sql_para.menu_double == sqlcfg->get_para()->menu_double && sql_para.menu_asset == sqlcfg->get_para()->menu_asset
            && sql_para.aa1_sql.camera == sqlcfg->get_para()->aa1_sql.camera && sql_para.aa2_sql.camera == sqlcfg->get_para()->aa2_sql.camera
            && sql_para.hfct1_sql.mode_recognition == sqlcfg->get_para()->hfct1_sql.mode_recognition
            && sql_para.hfct2_sql.mode_recognition == sqlcfg->get_para()->hfct2_sql.mode_recognition){
        flag = false;
    }
    sqlcfg->sql_save(&sql_para);

    //这些设置中，只有这几个量是需要写入FPGA才能生效的
    Common::write_fpga_offset_debug(data);

    qDebug()<<"debug para saved!";
    emit update_statusBar(tr("【调试模式】设置已保存！"));
    if(flag){
        QCoreApplication::quit();
    }
}

void DebugSet::reload()
{
    switch (sqlcfg->get_para()->menu_h1) {
    case TEV1:
        ui->lineEdit_H1_ZERO->setText(QString("%1").arg(sql_para.tev1_sql.fpga_zero ) );
        ui->lineEdit_H1_NOISE->setText(QString("%1").arg(sql_para.tev1_sql.offset_noise ) );
        ui->cbx_FIR_h1->setChecked(sql_para.tev1_sql.filter_fir_fpga);
        ui->cbx_wavelet_h1->setChecked(sql_para.tev1_sql.filter_wavelet);
        break;
    case HFCT1:
        ui->lineEdit_H1_ZERO->setText(QString("%1").arg(sql_para.hfct1_sql.fpga_zero ) );
        ui->rbt_bp_on_h1->setChecked(sql_para.hfct1_sql.mode_recognition);
        ui->rbt_bp_off_h1->setChecked(!sql_para.hfct1_sql.mode_recognition);
        ui->cbx_FIR_h1->setChecked(sql_para.hfct1_sql.filter_fir_fpga);
        ui->cbx_wavelet_h1->setChecked(sql_para.hfct1_sql.filter_wavelet);
        break;
    case UHF1:
        ui->lineEdit_H1_ZERO->setText(QString("%1").arg(sql_para.uhf1_sql.fpga_zero ) );
        ui->lineEdit_H1_NOISE->setText(QString("%1").arg(sql_para.uhf1_sql.offset_noise ) );
        ui->cbx_FIR_h1->setChecked(sql_para.uhf1_sql.filter_fir_fpga);
        ui->cbx_wavelet_h1->setChecked(sql_para.uhf1_sql.filter_wavelet);
        break;
    default:
        break;
    }
    switch (sqlcfg->get_para()->menu_h2) {
    case TEV2:
        ui->lineEdit_H2_ZERO->setText(QString("%1").arg(sql_para.tev2_sql.fpga_zero ) );
        ui->lineEdit_H2_NOISE->setText(QString("%1").arg(sql_para.tev2_sql.offset_noise ) );
        ui->cbx_FIR_h2->setChecked(sql_para.tev2_sql.filter_fir_fpga);
        ui->cbx_wavelet_h2->setChecked(sql_para.tev2_sql.filter_wavelet);
        break;
    case HFCT2:
        ui->lineEdit_H2_ZERO->setText(QString("%1").arg(sql_para.hfct2_sql.fpga_zero ) );
        ui->rbt_bp_on_h2->setChecked(sql_para.hfct2_sql.mode_recognition);
        ui->rbt_bp_off_h2->setChecked(!sql_para.hfct2_sql.mode_recognition);
        ui->cbx_FIR_h2->setChecked(sql_para.hfct2_sql.filter_fir_fpga);
        ui->cbx_wavelet_h2->setChecked(sql_para.hfct2_sql.filter_wavelet);
        break;
    case UHF2:
        ui->lineEdit_H2_ZERO->setText(QString("%1").arg(sql_para.uhf2_sql.fpga_zero ) );
        ui->lineEdit_H2_NOISE->setText(QString("%1").arg(sql_para.uhf2_sql.offset_noise ) );
        ui->cbx_FIR_h2->setChecked(sql_para.uhf2_sql.filter_fir_fpga);
        ui->cbx_wavelet_h2->setChecked(sql_para.uhf2_sql.filter_wavelet);
        break;
    default:
        break;
    }
    switch (sqlcfg->get_para()->menu_l1) {
    case AA1:
        ui->rbt_L1_camera_on->setChecked(sql_para.aa1_sql.camera);
        ui->rbt_L1_camera_off->setChecked(!sql_para.aa1_sql.camera);
        ui->lineEdit_L1_offset->setText(QString("%1").arg(sql_para.aa1_sql.offset_noise ) );
        ui->rbt_L1_envelope->setChecked(sql_para.aa1_sql.envelope);
        ui->rbt_L1_original->setChecked(!sql_para.aa1_sql.envelope);
        break;
    case AE1:
        ui->lineEdit_L1_Freq->setText(Common::sensor_freq_to_string(sql_para.ae1_sql.sensor_freq ) );
        ui->lineEdit_L1_offset->setText(QString("%1").arg(sql_para.ae1_sql.offset_noise ) );
        ui->rbt_L1_envelope->setChecked(sql_para.ae1_sql.envelope);
        ui->rbt_L1_original->setChecked(!sql_para.ae1_sql.envelope);
        break;
    default:
        break;
    }
    switch (sqlcfg->get_para()->menu_l2) {
    case AA2:
        ui->rbt_L2_camera_on->setChecked(sql_para.aa2_sql.camera);
        ui->rbt_L2_camera_off->setChecked(!sql_para.aa2_sql.camera);
        ui->lineEdit_L2_offset->setText(QString("%1").arg(sql_para.aa2_sql.offset_noise ) );
        ui->rbt_L2_envelope->setChecked(sql_para.aa2_sql.envelope);
        ui->rbt_L2_original->setChecked(!sql_para.aa2_sql.envelope);
        break;
    case AE2:
        ui->lineEdit_L2_Freq->setText(Common::sensor_freq_to_string(sql_para.ae2_sql.sensor_freq ) );
        ui->lineEdit_L2_offset->setText(QString("%1").arg(sql_para.ae2_sql.offset_noise ) );
        ui->rbt_L2_envelope->setChecked(sql_para.ae2_sql.envelope);
        ui->rbt_L2_original->setChecked(!sql_para.ae2_sql.envelope);
        break;
    default:
        break;
    }

    //通道选择
    switch (sql_para.menu_h1) {
    case TEV1:
        ui->rb_HC1_TEV->setChecked(true);
        break;
    case HFCT1:
        ui->rb_HC1_HFCT->setChecked(true);
        break;
    case UHF1:
        ui->rb_HC1_UHF->setChecked(true);
        break;
    case Disable:
        ui->rb_HC1_Disable->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para.menu_h2) {
    case TEV2:
        ui->rb_HC2_TEV->setChecked(true);
        break;
    case HFCT2:
        ui->rb_HC2_HFCT->setChecked(true);
        break;
    case UHF2:
        ui->rb_HC2_UHF->setChecked(true);
        break;
    case Disable:
        ui->rb_HC2_Disable->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para.menu_l1) {
    case AA1:
        ui->rb_LC1_AA->setChecked(true);
        break;
    case AE1:
        ui->rb_LC1_AE->setChecked(true);
        break;
    case Disable:
        ui->rb_LC1_Disable->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para.menu_l2) {
    case AA2:
        ui->rb_LC2_AA->setChecked(true);
        break;
    case AE2:
        ui->rb_LC2_AE->setChecked(true);
        break;
    case Disable:
        ui->rb_LC2_Disable->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para.menu_double) {
    case Double_Channel:
        ui->rb_Double_Enable->setChecked(true);
        break;
    case Disable:
        ui->rb_Double_Disable->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para.menu_asset) {
    case ASSET:
        ui->rb_Asset_Enable->setChecked(true);
        break;
    case Disable:
        ui->rb_Asset_Disable->setChecked(true);
        break;
    default:
        break;
    }
}

void DebugSet::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;
    this->show();
}

void DebugSet::trans_key(quint8 key_code)
{
    if (key_val == NULL || key_val->grade.val0 != 6 || key_val->grade.val1 != 2) {
        return;
    }

    if(key_val->grade.val5 != 0){
        emit send_key(key_code);
        return;
    }

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

                reload();
                key_val->grade.val3 = 1;
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

void DebugSet::do_key_ok()
{
    if(key_val->grade.val3 == 4 && key_val->grade.val4 == 1){
        //插入保存参数文件代码
        QDir dir;
        dir.setPath("/root/OEM");
        if(dir.removeRecursively()){
            ui->label_channel_data_tips->setText(tr("OEM文件已重置"));
            system("sync");
        }
        else{
            ui->label_channel_data_tips->setText(tr("OEM文件未重置"));
        }
        //                ui->label_channel_data_tips->setText(tr("通道参数已保存至文件"));
    }
    else{
        if(chk_status_h1 != 00 || chk_status_h2 != 00){            //切换复选框状态
            if(chk_status_h1 == 10){
                switch (sql_para.menu_h1) {
                case TEV1:
                    sql_para.tev1_sql.filter_fir_fpga = !sql_para.tev1_sql.filter_fir_fpga;
                    break;
                case HFCT1:
                    sql_para.hfct1_sql.filter_fir_fpga = !sql_para.hfct1_sql.filter_fir_fpga;
                    break;
                case UHF1:
                    sql_para.uhf1_sql.filter_fir_fpga = !sql_para.uhf1_sql.filter_fir_fpga;
                    break;
                default:
                    break;
                }
            }
            else if(chk_status_h1 == 01){
                switch (sql_para.menu_h1) {
                case TEV1:
                    sql_para.tev1_sql.filter_wavelet = !sql_para.tev1_sql.filter_wavelet;
                    break;
                case HFCT1:
                    sql_para.hfct1_sql.filter_wavelet = !sql_para.hfct1_sql.filter_wavelet;
                    break;
                case UHF1:
                    sql_para.uhf1_sql.filter_wavelet = !sql_para.uhf1_sql.filter_wavelet;
                    break;
                default:
                    break;
                }
            }
            else if(chk_status_h2 == 10){
                switch (sql_para.menu_h2) {
                case TEV2:
                    sql_para.tev2_sql.filter_fir_fpga = !sql_para.tev2_sql.filter_fir_fpga;
                    break;
                case HFCT2:
                    sql_para.hfct2_sql.filter_fir_fpga = !sql_para.hfct2_sql.filter_fir_fpga;
                    break;
                case UHF2:
                    sql_para.uhf2_sql.filter_fir_fpga = !sql_para.uhf2_sql.filter_fir_fpga;
                    break;
                default:
                    break;
                }
            }
            else if(chk_status_h2 == 01){
                switch (sql_para.menu_h2) {
                case TEV2:
                    sql_para.tev2_sql.filter_wavelet = !sql_para.tev2_sql.filter_wavelet;
                    break;
                case HFCT2:
                    sql_para.hfct2_sql.filter_wavelet = !sql_para.hfct2_sql.filter_wavelet;
                    break;
                case UHF2:
                    sql_para.uhf2_sql.filter_wavelet = !sql_para.uhf2_sql.filter_wavelet;
                    break;
                default:
                    break;
                }
            }
        }
        saveSql();                          //保存
    }
}

void DebugSet::do_key_cancel()
{
    if(key_val->grade.val4 == 0){   //退出Debug设置
        key_val->grade.val3 = 0;
        key_val->grade.val2 = 0;

        this->hide();
        resetPassword();
        fresh_parent();
    }
    else{
        key_val->grade.val4 = 0;    //退出三级菜单
        chk_status_h1 = 00;
        chk_status_h2 = 00;
    }
}

void DebugSet::do_key_left_right(int d)
{
    if(key_val->grade.val4 == 0 && key_val->grade.val3 != 0 && d > 0){   //必须第三层处于工作状态
        key_val->grade.val4 =1;
    }
    else{
        QList<int> list;
        switch (key_val->grade.val3) {
        case 1:         //高频
            switch (key_val->grade.val4) {
            case 1:         //H1零偏置
                switch (sqlcfg->get_para()->menu_h1) {
                case TEV1:
                    sql_para.tev1_sql.fpga_zero += d;
                    break;
                case HFCT1:
                    sql_para.hfct1_sql.fpga_zero += d;
                    break;
                case UHF1:
                    sql_para.uhf1_sql.fpga_zero += d;
                    break;
                default:
                    break;
                }
                break;
            case 2:         //H1噪声偏置
                switch (sqlcfg->get_para()->menu_h1) {
                case TEV1:
                    sql_para.tev1_sql.offset_noise += d;
                    break;
                case HFCT1:
                    //                    sql_para.hfct1_sql.offset_noise += d;
                    sql_para.hfct1_sql.mode_recognition = !sql_para.hfct1_sql.mode_recognition;     //模式识别
                    break;
                case UHF1:
                    sql_para.uhf1_sql.offset_noise += d;
                    break;
                default:
                    break;
                }
                break;
            case 3:         //H1前置滤波
                Common::change_value(chk_status_h1, 01, 10);
                break;
            case 4:         //H2零偏置
                switch (sqlcfg->get_para()->menu_h2) {
                case TEV2:
                    sql_para.tev2_sql.fpga_zero += d;
                    break;
                case HFCT2:
                    sql_para.hfct2_sql.fpga_zero += d;
                    break;
                case UHF2:
                    sql_para.uhf2_sql.fpga_zero += d;
                    break;
                default:
                    break;
                }
                break;
            case 5:         //H2噪声偏置
                switch (sqlcfg->get_para()->menu_h2) {
                case TEV2:
                    sql_para.tev2_sql.offset_noise += d;
                    break;
                case HFCT2:
                    //                    sql_para.hfct2_sql.offset_noise += d;
                    sql_para.hfct2_sql.mode_recognition = !sql_para.hfct2_sql.mode_recognition;     //模式识别
                    break;
                case UHF2:
                    sql_para.uhf2_sql.offset_noise += d;
                    break;
                default:
                    break;
                }
                break;
            case 6:         //H2前置滤波
                Common::change_value(chk_status_h2, 01, 10);
                break;
            default:
                break;
            }
            break;
        case 2:     //低频
            switch (key_val->grade.val4) {            
            case 1:     //L1噪声
                if(sqlcfg->get_para()->menu_l1 == AA1){
                    sql_para.aa1_sql.offset_noise += d;
                }
                else if(sqlcfg->get_para()->menu_l1 == AE1){
                    sql_para.ae1_sql.offset_noise += d;
                }
                break;
            case 2:     //L1摄像头/传感器中心频率
                if(sqlcfg->get_para()->menu_l1 == AA1){
                    sql_para.aa1_sql.camera = !sql_para.aa1_sql.camera;
                    if(sql_para.aa1_sql.chart == Camera && sql_para.aa1_sql.camera == false){       //关闭摄像头，把显示界面重置
                        sql_para.aa1_sql.chart = BASIC;
                    }
                }
                else if(sqlcfg->get_para()->menu_l1 == AE1){
                    list.clear();
                    list << ae_factor_30k << ae_factor_40k << ae_factor_50k << ae_factor_60k
                         << ae_factor_70k << ae_factor_80k << ae_factor_90k << ae_factor_30k_v2 << ae_factor_40k_v2;
                    Common::change_index(sql_para.ae1_sql.sensor_freq, d, list);
                }
                break;
            case 3:     //L1包络线
                if(sqlcfg->get_para()->menu_l1 == AA1){
                    sql_para.aa1_sql.envelope = !sql_para.aa1_sql.envelope;
                }
                else if(sqlcfg->get_para()->menu_l1 == AE1){
                    sql_para.ae1_sql.envelope = !sql_para.ae1_sql.envelope;
                }
                break;            
            case 4:     //L2噪声
                if(sqlcfg->get_para()->menu_l2 == AA2){
                    sql_para.aa2_sql.offset_noise += d;
                }
                else if(sqlcfg->get_para()->menu_l2 == AE2){
                    sql_para.ae2_sql.offset_noise += d;
                }
                break;
            case 5:     //L2摄像头/传感器中心频率
                if(sqlcfg->get_para()->menu_l2 == AA2){
                    sql_para.aa2_sql.camera = !sql_para.aa2_sql.camera;
                    if(sql_para.aa2_sql.chart == Camera && sql_para.aa2_sql.camera == false){       //关闭摄像头，把显示界面重置
                        sql_para.aa2_sql.chart = BASIC;
                    }
                }
                else if(sqlcfg->get_para()->menu_l2 == AE2){
                    list.clear();
                    list << ae_factor_30k << ae_factor_40k << ae_factor_50k << ae_factor_60k
                         << ae_factor_70k << ae_factor_80k << ae_factor_90k << ae_factor_30k_v2 << ae_factor_40k_v2;
                    Common::change_index(sql_para.ae2_sql.sensor_freq, d, list);
                }
                break;
            case 6:     //L2包络线
                if(sqlcfg->get_para()->menu_l2 == AA2){
                    sql_para.aa2_sql.envelope = !sql_para.aa2_sql.envelope;
                }
                else if(sqlcfg->get_para()->menu_l2 == AE2){
                    sql_para.ae2_sql.envelope = !sql_para.ae2_sql.envelope;
                }
                break;
            default:
                break;
            }
            break;
        case 3:     //通道
            list.clear();
            switch (key_val->grade.val4) {
            case 1:     //高频通道1
                list << Disable << TEV1 << HFCT1 << UHF1;
                Common::change_index(sql_para.menu_h1, d, list);
                break;
            case 2:
                list << Disable << TEV2 << HFCT2 << UHF2;
                Common::change_index(sql_para.menu_h2, d, list);
                break;
            case 3:
                list << Disable << AA1 << AE1;
                Common::change_index(sql_para.menu_l1, d, list);
                break;
            case 4:
                list << Disable << AA2 << AE2;
                Common::change_index(sql_para.menu_l2, d, list);
                break;
            case 5:
                list << Disable << Double_Channel;
                Common::change_index(sql_para.menu_double, d, list);
                break;
            case 6:
                list << Disable << ASSET;
                Common::change_index(sql_para.menu_asset, d, list);
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

void DebugSet::do_key_up_down(int d)
{
    if(key_val->grade.val4 == 0){   //判断在二级菜单
        Common::change_index(key_val->grade.val3, d, SETTING_NUM, 1);
    }
    else{                           //判断在三级菜单
        switch (key_val->grade.val3) {
        case 1:                     //高频
            Common::change_index(key_val->grade.val4, d, SETTING_NUM_HIGH, 1);
            switch (key_val->grade.val4) {
            case 3:
                chk_status_h1 = 10;
                chk_status_h2 = 00;
                break;
            case 6:
                chk_status_h1 = 00;
                chk_status_h2 = 10;
                break;
            default:
                chk_status_h1 = 00;
                chk_status_h2 = 00;
                break;
            }
            break;
        case 2:                     //低频
            Common::change_index(key_val->grade.val4, d, SETTING_NUM_LOW, 1);
            break;
        case 3:                     //通道
            Common::change_index(key_val->grade.val4, d, SETTING_NUM_CHANNEL, 1);
            break;
        default:
            break;
        }
    }
}

void DebugSet::fresh_rdb_data()
{
    switch (sqlcfg->get_para()->menu_h1) {
    case TEV1:
        ui->lab_offset_zero_h1->setText(QString("%1").arg(Common::rdb_get_yc_value(TEV1_center_biased_adv_yc)));
        ui->lab_offset_noise_h1->setText(QString("%1").arg(Common::rdb_get_yc_value(TEV1_noise_biased_adv_yc)));;
        break;
    case HFCT1:
        ui->lab_offset_zero_h1->setText(QString("%1").arg(Common::rdb_get_yc_value(HFCT1_center_biased_adv_yc)));
        ui->lab_offset_noise_h1->setText(QString("%1").arg(Common::rdb_get_yc_value(HFCT1_noise_biased_adv_yc)));;
        break;
    case UHF1:
        ui->lab_offset_zero_h1->setText(QString("%1").arg(Common::rdb_get_yc_value(UHF1_center_biased_adv_yc)));
        ui->lab_offset_noise_h1->setText(QString("%1").arg(Common::rdb_get_yc_value(UHF1_noise_biased_adv_yc)));;
        break;
    default:
        break;
    }

    switch (sqlcfg->get_para()->menu_h2) {
    case TEV2:
        ui->lab_offset_zero_h2->setText(QString("%1").arg(Common::rdb_get_yc_value(TEV2_center_biased_adv_yc)));
        ui->lab_offset_noise_h2->setText(QString("%1").arg(Common::rdb_get_yc_value(TEV2_noise_biased_adv_yc)));;
        break;
    case HFCT2:
        ui->lab_offset_zero_h2->setText(QString("%1").arg(Common::rdb_get_yc_value(HFCT2_center_biased_adv_yc)));
        ui->lab_offset_noise_h2->setText(QString("%1").arg(Common::rdb_get_yc_value(HFCT2_noise_biased_adv_yc)));;
        break;
    case UHF2:
        ui->lab_offset_zero_h2->setText(QString("%1").arg(Common::rdb_get_yc_value(UHF2_center_biased_adv_yc)));
        ui->lab_offset_noise_h2->setText(QString("%1").arg(Common::rdb_get_yc_value(UHF2_noise_biased_adv_yc)));;
        break;
    default:
        break;
    }

    switch (sqlcfg->get_para()->menu_l1) {
    case AA1:
        ui->lab_offset_noise_L1->setText(QString("%1").arg(Common::rdb_get_yc_value(AA1_noise_biased_adv_yc)));
        break;
    case AE1:
        ui->lab_offset_noise_L1->setText(QString("%1").arg(Common::rdb_get_yc_value(AE1_noise_biased_adv_yc)));
        break;
    default:
        break;
    }
    switch (sqlcfg->get_para()->menu_l2) {
    case AA2:
        ui->lab_offset_noise_L2->setText(QString("%1").arg(Common::rdb_get_yc_value(AA2_noise_biased_adv_yc)));
        break;
    case AE2:
        ui->lab_offset_noise_L2->setText(QString("%1").arg(Common::rdb_get_yc_value(AE2_noise_biased_adv_yc)));
        break;
    default:
        break;
    }

    ui->lineEdit_CPU_TEMP->setText(QString("%1").arg(Common::rdb_get_yc_value(CPU_temp_yc)) );
    ui->lineEdit_CPU_VCC->setText(QString("%1").arg(Common::rdb_get_yc_value(CPU_vcc_yc)) );
    float v = Common::rdb_get_yc_value(Battery_vcc_yc), c = Common::rdb_get_yc_value(Battery_cur_yc);
    ui->lineEdit_BATT_VCC->setText(QString("%1").arg(v) );
    ui->lineEdit_BATT_CUR->setText(QString("%1").arg(c) );
    ui->lineEdit_BATT_P->setText(QString("%1").arg(v*c/1000) );
//    float v = battery->battVcc(), c = battery->battCur();
//    ui->lineEdit_BATT_VCC->setText(QString("%1").arg(v) );
//    ui->lineEdit_BATT_CUR->setText(QString("%1").arg(c) );
//    ui->lineEdit_BATT_P->setText(QString("%1").arg(v*c/1000) );
}

void DebugSet::fresh()
{
    if(pass){
        if(key_val->grade.val3){
            ui->tabWidget->setCurrentIndex(key_val->grade.val3-1);
        }

        //刷新数据
        reload();

        //刷新组件状态
        tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
        tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
        tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
        tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
        if(chk_status_h1 == 00){
            ui->cbx_FIR_h1->setStyleSheet("");
            ui->cbx_wavelet_h1->setStyleSheet("");
        }
        else if(chk_status_h1 == 10){
            ui->cbx_FIR_h1->setStyleSheet("QCheckBox{background-color:darkGray;}");
            ui->cbx_wavelet_h1->setStyleSheet("");
        }
        else if(chk_status_h1 == 01){
            ui->cbx_FIR_h1->setStyleSheet("");
            ui->cbx_wavelet_h1->setStyleSheet("QCheckBox{background-color:darkGray;}");
        }
        if(chk_status_h2 == 00){
            ui->cbx_FIR_h2->setStyleSheet("");
            ui->cbx_wavelet_h2->setStyleSheet("");
        }
        else if(chk_status_h2 == 10){
            ui->cbx_FIR_h2->setStyleSheet("QCheckBox{background-color:darkGray;}");
            ui->cbx_wavelet_h2->setStyleSheet("");
        }
        else if(chk_status_h2 == 01){
            ui->cbx_FIR_h2->setStyleSheet("");
            ui->cbx_wavelet_h2->setStyleSheet("QCheckBox{background-color:darkGray;}");
        }

        switch (key_val->grade.val3) {
        case 1:         //高频
            ui->lineEdit_H1_ZERO->deselect();
            ui->lineEdit_H1_NOISE->deselect();
            ui->lineEdit_H2_ZERO->deselect();
            ui->lineEdit_H2_NOISE->deselect();
            ui->rbt_bp_on_h1->setStyleSheet("");
            ui->rbt_bp_off_h1->setStyleSheet("");
            ui->rbt_bp_on_h2->setStyleSheet("");
            ui->rbt_bp_off_h2->setStyleSheet("");
            switch (key_val->grade.val4) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            case 1:
                ui->lineEdit_H1_ZERO->selectAll();
                break;
            case 2:
                ui->lineEdit_H1_NOISE->selectAll();
                Common::change_rbt_status(sql_para.hfct1_sql.mode_recognition, ui->rbt_bp_on_h1, ui->rbt_bp_off_h1);
                break;
            case 4:
                ui->lineEdit_H2_ZERO->selectAll();
                break;
            case 5:
                ui->lineEdit_H2_NOISE->selectAll();
                Common::change_rbt_status(sql_para.hfct2_sql.mode_recognition, ui->rbt_bp_on_h2, ui->rbt_bp_off_h2);
                break;
            default:
                break;
            }
            break;
        case 2:         //低频
            ui->lineEdit_L1_offset->deselect();
            ui->lineEdit_L2_offset->deselect();
            ui->lineEdit_L1_Freq->deselect();
            ui->lineEdit_L2_Freq->deselect();
            ui->rbt_L1_camera_off->setStyleSheet("");
            ui->rbt_L1_camera_on->setStyleSheet("");
            ui->rbt_L2_camera_off->setStyleSheet("");
            ui->rbt_L2_camera_on->setStyleSheet("");
            ui->rbt_L1_envelope->setStyleSheet("");
            ui->rbt_L1_original->setStyleSheet("");
            ui->rbt_L2_envelope->setStyleSheet("");
            ui->rbt_L2_original->setStyleSheet("");
            switch (key_val->grade.val4) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            case 1:
                ui->lineEdit_L1_offset->selectAll();
                break;
            case 2:
                Common::change_rbt_status(sql_para.aa1_sql.camera, ui->rbt_L1_camera_on, ui->rbt_L1_camera_off);
                ui->lineEdit_L1_Freq->selectAll();
                break;
            case 3:
                if(sqlcfg->get_para()->menu_l1 == AA1){
                    Common::change_rbt_status(sql_para.aa1_sql.envelope, ui->rbt_L1_envelope, ui->rbt_L1_original);
                }
                else if(sqlcfg->get_para()->menu_l1 == AE1){
                    Common::change_rbt_status(sql_para.ae1_sql.envelope, ui->rbt_L1_envelope, ui->rbt_L1_original);
                }
                break;
            case 4:
                ui->lineEdit_L2_offset->selectAll();
                break;
            case 5:
                Common::change_rbt_status(sql_para.aa2_sql.camera, ui->rbt_L2_camera_on, ui->rbt_L2_camera_off);
                ui->lineEdit_L2_Freq->selectAll();
                break;
            case 6:
                if(sqlcfg->get_para()->menu_l2 == AA2){
                    Common::change_rbt_status(sql_para.aa2_sql.envelope, ui->rbt_L2_envelope, ui->rbt_L2_original);
                }
                else if(sqlcfg->get_para()->menu_l2 == AE2){
                    Common::change_rbt_status(sql_para.ae2_sql.envelope, ui->rbt_L2_envelope, ui->rbt_L2_original);
                }
                break;
            default:
                break;
            }
            break;
        case 3:         //通道
            ui->pbt_HC1->setChecked(false);
            ui->pbt_HC2->setChecked(false);
            ui->pbt_LC1->setChecked(false);
            ui->pbt_LC2->setChecked(false);
            ui->pbt_Double->setChecked(false);
            ui->pbt_Asset->setChecked(false);
            switch (key_val->grade.val4) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            case 1:
                ui->pbt_HC1->setChecked(true);
                break;
            case 2:
                ui->pbt_HC2->setChecked(true);
                break;
            case 3:
                ui->pbt_LC1->setChecked(true);
                break;
            case 4:
                ui->pbt_LC2->setChecked(true);
                break;
            case 5:
                ui->pbt_Double->setChecked(true);
                break;
            case 6:
                ui->pbt_Asset->setChecked(true);
                break;
            default:
                break;
            }
            break;
        case 4:         //状态监测
            tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab3->setStyleSheet("QLabel{border: 1px solid darkGray;}");
            if(key_val->grade.val4 == 1){
                ui->pbt_channel_data->setStyleSheet("QPushButton {background-color:gray;}");
            }
            else{
                ui->pbt_channel_data->setStyleSheet("");
            }
            break;
        default:
            break;
        }
    }
}




