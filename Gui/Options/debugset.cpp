#include "debugset.h"
#include "ui_debugui.h"
#include <QButtonGroup>
#include <QDir>
#include "../Common/common.h"
#include "IO/Com/rdb/rdb.h"

#include <sys/time.h>    // for gettimeofday()

#define SETTING_NUM             6           //设置菜单条目数
#define SETTING_NUM_TEV         6
#define SETTING_NUM_HFCT        4
#define SETTING_NUM_AA          6
#define SETTING_NUM_AE          6
#define SETTING_NUM_CHANNEL     5
#define THRESHOLD_STEP          10

DebugSet::DebugSet(G_PARA *g_data,QWidget *parent) : QFrame(parent),ui(new Ui::DebugUi)
{
    key_val = NULL;
    data = g_data;
    sql_para = *sqlcfg->get_para();

    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    ui->setupUi(this);

    cpu_status = new CPUStatus;
    battery = new Battery;
    timer_rdb = new QTimer;
    timer_rdb->start(1000);
    connect(timer_rdb, SIGNAL(timeout()), this, SLOT(fresh_rdb_data()) );

    timer_hardware = new QTimer;
    timer_hardware->start(5000);        //5秒检测一次硬件及同步
//    timer_hardware->start(20000);
    connect(timer_hardware, SIGNAL(timeout()), this, SLOT(fresh_hardware_status()) );

    timer_sync = new QTimer();
    timer_sync->setInterval(1);       //1毫秒1跳
    connect(timer_sync, SIGNAL(timeout()), this, SLOT(fresh_sync()) );

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
    tab0 = new QLabel(tr("地电波"),ui->tabWidget->tabBar());
    Common::setTab(tab0);
    tab1 = new QLabel(tr("高频CT"),ui->tabWidget->tabBar());
    Common::setTab(tab1);
    tab2 = new QLabel(tr("AA超声"),ui->tabWidget->tabBar());
    Common::setTab(tab2);
    tab3 = new QLabel(tr("AE超声"),ui->tabWidget->tabBar());
    Common::setTab(tab3);
    tab4 = new QLabel(tr("通道设置"),ui->tabWidget->tabBar());
    Common::setTab(tab4);
    tab5 = new QLabel(tr("硬件监测"),ui->tabWidget->tabBar());
    Common::setTab(tab5);

    ui->tabWidget->tabBar()->setTabButton(0,QTabBar::LeftSide,tab0);
    ui->tabWidget->tabBar()->setTabButton(1,QTabBar::LeftSide,tab1);
    ui->tabWidget->tabBar()->setTabButton(2,QTabBar::LeftSide,tab2);
    ui->tabWidget->tabBar()->setTabButton(3,QTabBar::LeftSide,tab3);
    ui->tabWidget->tabBar()->setTabButton(4,QTabBar::LeftSide,tab4);
    ui->tabWidget->tabBar()->setTabButton(5,QTabBar::LeftSide,tab5);

    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        ui->tabWidget->widget(i)->setStyleSheet("QWidget {background-color:lightGray;}");
    }

    QString style = "QLabel {font-family:WenQuanYi Micro Hei;font: bold; font-size:16px;color:green}";
    switch (sqlcfg->get_para()->menu_h1) {
    case TEV1:
        ui->label_TEV1->setStyleSheet(style);
        break;
    case HFCT1:
        ui->label_HF1->setStyleSheet(style);
        break;
    default:
        break;
    }
    switch (sqlcfg->get_para()->menu_h2) {
    case TEV2:
        ui->label_TEV2->setStyleSheet(style);
        break;
    case HFCT2:
        ui->label_HF2->setStyleSheet(style);
        break;
    default:
        break;
    }
    switch (sqlcfg->get_para()->menu_l1) {
    case AA1:
        ui->label_AA1->setStyleSheet(style);
        break;
    case AE1:
        ui->label_AE1->setStyleSheet(style);
        break;
    default:
        break;
    }
    switch (sqlcfg->get_para()->menu_l2) {
    case AA2:
        ui->label_AA2->setStyleSheet(style);
        break;
    case AE2:
        ui->label_AE2->setStyleSheet(style);
        break;
    default:
        break;
    }

    QButtonGroup *group1 = new QButtonGroup(this);
    QButtonGroup *group2 = new QButtonGroup(this);
    QButtonGroup *group3 = new QButtonGroup(this);
    QButtonGroup *group4 = new QButtonGroup(this);
    QButtonGroup *group5 = new QButtonGroup(this);
    QButtonGroup *group6 = new QButtonGroup(this);
    QButtonGroup *group7 = new QButtonGroup(this);
    QButtonGroup *group8 = new QButtonGroup(this);
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
    group5->addButton( ui->rbt_AA1_envelope );
    group5->addButton( ui->rbt_AA1_original );
    group6->addButton( ui->rbt_AA2_envelope );
    group6->addButton( ui->rbt_AA2_original );
    group7->addButton( ui->rbt_AE1_envelope );
    group7->addButton( ui->rbt_AE1_original );
    group8->addButton( ui->rbt_AE2_envelope );
    group8->addButton( ui->rbt_AE2_original );
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
            && sql_para.menu_double == sqlcfg->get_para()->menu_double){
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
    //TEV
    ui->lineEdit_TEV1_line->setText(QString("%1 mV").arg((int)Common::physical_value(sql_para.tev1_sql.offset_linearity, TEV1) ) );
//    ui->lineEdit_TEV1_ZERO->setText(QString("%1").arg((int)Common::physical_value(sql_para.tev1_sql.fpga_zero, TEV1) ) );
//    ui->lineEdit_TEV1_NOISE->setText(QString("%1").arg((int)Common::physical_value(sql_para.tev1_sql.offset_noise, TEV1) ) );
    ui->lineEdit_TEV1_ZERO->setText(QString("%1").arg(sql_para.tev1_sql.fpga_zero ) );
    ui->lineEdit_TEV1_NOISE->setText(QString("%1").arg(sql_para.tev1_sql.offset_noise ) );

    ui->lineEdit_TEV2_line->setText(QString("%1 mV").arg((int)Common::physical_value(sql_para.tev2_sql.offset_linearity, TEV2) ) );
//    ui->lineEdit_TEV2_ZERO->setText(QString("%1").arg((int)Common::physical_value(sql_para.tev2_sql.fpga_zero, TEV2) ) );
//    ui->lineEdit_TEV2_NOISE->setText(QString("%1").arg((int)Common::physical_value(sql_para.tev2_sql.offset_noise, TEV2) ) );
    ui->lineEdit_TEV2_ZERO->setText(QString("%1").arg(sql_para.tev2_sql.fpga_zero ) );
    ui->lineEdit_TEV2_NOISE->setText(QString("%1").arg(sql_para.tev2_sql.offset_noise ) );

    //HFCT
    ui->lineEdit_HFCT1_THRESHOLD->setText(QString("%1 mV").arg((int)Common::physical_value(sql_para.hfct1_sql.fpga_threshold, HFCT1) ) );
    ui->lineEdit_HFCT1_ZERO->setText(QString("%1").arg(sql_para.hfct1_sql.fpga_zero) );

    ui->lineEdit_HFCT2_THRESHOLD->setText(QString("%1 mV").arg((int)Common::physical_value(sql_para.hfct2_sql.fpga_threshold, HFCT2) ) );
    ui->lineEdit_HFCT2_ZERO->setText(QString("%1").arg(sql_para.hfct2_sql.fpga_zero) );

    //AA
    ui->lineEdit_AA1_Step->setText(QString("%1").arg(sql_para.aa1_sql.step));
    ui->lineEdit_AA1_offset->setText(QString("%1").arg(sql_para.aa1_sql.offset));
    ui->rbt_AA1_envelope->setChecked(sql_para.aa1_sql.envelope);
    ui->rbt_AA1_original->setChecked(!sql_para.aa1_sql.envelope);

    ui->lineEdit_AA2_Step->setText(QString("%1").arg(sql_para.aa2_sql.step));
    ui->lineEdit_AA2_offset->setText(QString("%1").arg(sql_para.aa2_sql.offset));
    ui->rbt_AA2_envelope->setChecked(sql_para.aa2_sql.envelope);
    ui->rbt_AA2_original->setChecked(!sql_para.aa2_sql.envelope);

    //AE
    ui->lineEdit_AE1_Step->setText(QString("%1").arg(sql_para.ae1_sql.step));
    ui->lineEdit_AE1_offset->setText(QString("%1").arg(sql_para.ae1_sql.offset));
    ui->rbt_AE1_envelope->setChecked(sql_para.ae1_sql.envelope);
    ui->rbt_AE1_original->setChecked(!sql_para.ae1_sql.envelope);

    ui->lineEdit_AE2_Step->setText(QString("%1").arg(sql_para.ae2_sql.step));
    ui->lineEdit_AE2_offset->setText(QString("%1").arg(sql_para.ae2_sql.offset));
    ui->rbt_AE2_envelope->setChecked(sql_para.ae2_sql.envelope);
    ui->rbt_AE2_original->setChecked(!sql_para.ae2_sql.envelope);

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

    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val1 != 2) {
        return;
    }
    if(key_val->grade.val2 != 1){
        return;
    }
    if(key_val->grade.val5 != 0){
        emit send_key(key_code);
        return;
    }

    switch (key_code) {
    case KEY_OK:
        if(pass){
            saveSql();                          //保存
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
        if(key_val->grade.val4 == 0){   //退出Debug设置
            key_val->grade.val3 = 0;
            key_val->grade.val2 = 0;

            this->hide();
            resetPassword();
            fresh_parent();
        }
        else{
            key_val->grade.val4 = 0;    //退出三级菜单
        }

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

void DebugSet::do_key_left_right(int d)
{
    if(key_val->grade.val4 == 0 && key_val->grade.val3 != 0 && d > 0){   //必须第三层处于工作状态
        key_val->grade.val4 =1;
    }
    else{
        QList<int> list;

        switch (key_val->grade.val3) {
        case 1:         //TEV
            switch (key_val->grade.val4) {
            case 1:
//                sql_para.tev1_sql.fpga_threshold += Common::code_value(1,TEV1) * d;
                sql_para.tev1_sql.offset_linearity += Common::code_value(1,TEV1) * d;
                break;
            case 2:
                sql_para.tev1_sql.fpga_zero += d;
//                sql_para.tev1_sql.fpga_zero += Common::code_value(1,TEV1) * d;
                break;
            case 3:
                sql_para.tev1_sql.offset_noise += d;
//                sql_para.tev1_sql.offset_noise += Common::code_value(1,TEV1) * d;
                break;
            case 4:
//                sql_para.tev2_sql.fpga_threshold += Common::code_value(1,TEV2) * d;
                sql_para.tev2_sql.offset_linearity += Common::code_value(1,TEV2) * d;
                break;
            case 5:
                sql_para.tev2_sql.fpga_zero += d;
//                sql_para.tev2_sql.fpga_zero += Common::code_value(1,TEV2) * d;
                break;
            case 6:
                sql_para.tev2_sql.offset_noise += d;
//                sql_para.tev2_sql.offset_noise += Common::code_value(1,TEV2) * d;
                break;
            default:
                break;
            }
            break;
        case 2:         //HFCT
            switch (key_val->grade.val4) {
            case 1:
                sql_para.hfct1_sql.fpga_threshold += Common::code_value(1,HFCT1) * d;
                break;
            case 2:
                sql_para.hfct1_sql.fpga_zero += d;
                break;
            case 3:
                sql_para.hfct2_sql.fpga_threshold += Common::code_value(1,HFCT1) * d;
                break;
            case 4:
                sql_para.hfct2_sql.fpga_zero += d;
                break;
            default:
                break;
            }
            break;
        case 3:     //AA
            switch (key_val->grade.val4) {
            case 1:
                Common::change_index(sql_para.aa1_sql.step, 0.5 * d, 10, 0.5);
                break;
            case 2:
                sql_para.aa1_sql.offset += d;
                break;
            case 3:
                sql_para.aa1_sql.envelope = !sql_para.aa1_sql.envelope;
                break;
            case 4:
                Common::change_index(sql_para.aa2_sql.step, 0.5 * d, 10, 0.5);
                break;
            case 5:
                sql_para.aa2_sql.offset += d;
                break;
            case 6:
                sql_para.aa2_sql.envelope = !sql_para.aa2_sql.envelope;
                break;
            default:
                break;
            }
            break;
        case 4:     //AE
            switch (key_val->grade.val4) {
            case 1:
                Common::change_index(sql_para.ae1_sql.step, 0.5 * d, 10, 0.5);
                break;
            case 2:
                sql_para.ae1_sql.offset += d;
                break;
            case 3:
                sql_para.ae1_sql.envelope = !sql_para.ae1_sql.envelope;
                break;
            case 4:
                Common::change_index(sql_para.ae2_sql.step, 0.5 * d, 10, 0.5);
                break;
            case 5:
                sql_para.ae2_sql.offset += d;
                break;
            case 6:
                sql_para.ae2_sql.envelope = !sql_para.ae2_sql.envelope;
                break;
            default:
                break;
            }
            break;
        case 5:     //通道
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
        case 1:                     //TEV
            Common::change_index(key_val->grade.val4, d, SETTING_NUM_TEV, 1);
            break;
        case 2:                     //HFCT
            Common::change_index(key_val->grade.val4, d, SETTING_NUM_HFCT, 1);
            break;
        case 3:                     //AA
            Common::change_index(key_val->grade.val4, d, SETTING_NUM_AA, 1);
            break;
        case 4:                     //AE
            Common::change_index(key_val->grade.val4, d, SETTING_NUM_AE, 1);
            break;
        case 5:                     //通道
            Common::change_index(key_val->grade.val4, d, SETTING_NUM_CHANNEL, 1);
            break;
        default:
            break;
        }
    }
}

void DebugSet::fresh_rdb_data()
{
    yc_data_type temp_data;
    unsigned char a[1],b[1];

    yc_get_value(0,TEV1_center_biased_adv,1, &temp_data, b, a);
    ui->lab_offset_zero_tev1->setText(QString("%1").arg(temp_data.f_val));
    yc_get_value(0,TEV1_noise_biased_adv,1, &temp_data, b, a);
    ui->lab_offset_noise_tev1->setText(QString("%1").arg(temp_data.f_val));;

    yc_get_value(0,TEV2_center_biased_adv,1, &temp_data, b, a);
    ui->lab_offset_zero_tev2->setText(QString("%1").arg(temp_data.f_val));
    yc_get_value(0,TEV2_noise_biased_adv,1, &temp_data, b, a);
    ui->lab_offset_noise_tev2->setText(QString("%1").arg(temp_data.f_val));;

    yc_get_value(0,HFCT1_center_biased_adv,1, &temp_data, b, a);
    ui->lab_offset_zero_hfct1->setText(QString("%1").arg(temp_data.f_val));

    yc_get_value(0,HFCT2_center_biased_adv,1, &temp_data, b, a);
    ui->lab_offset_zero_hfct2->setText(QString("%1").arg(temp_data.f_val));

    yc_get_value(0,AA1_biased_adv,1, &temp_data, b, a);
    ui->lab_offset_noise_aa1->setText(QString("%1").arg(temp_data.f_val));
    yc_get_value(0,AA2_biased_adv,1, &temp_data, b, a);
    ui->lab_offset_noise_aa2->setText(QString("%1").arg(temp_data.f_val));

    yc_get_value(0,AE1_biased_adv,1, &temp_data, b, a);
    ui->lab_offset_noise_ae1->setText(QString("%1").arg(temp_data.f_val));
    yc_get_value(0,AE2_biased_adv,1, &temp_data, b, a);
    ui->lab_offset_noise_ae2->setText(QString("%1").arg(temp_data.f_val));
}

void DebugSet::fresh_hardware_status()
{
    float temp, vcc, sync;
    cpu_status->get_cpu_temp(&temp);
    cpu_status->get_cpu_vcc(&vcc);
    ui->lineEdit_CPU_TEMP->setText(QString("%1").arg(temp) );
    ui->lineEdit_CPU_VCC->setText(QString("%1").arg(vcc) );
    float v = battery->battVcc(), c = battery->battCur();
    ui->lineEdit_BATT_VCC->setText(QString("%1").arg(v) );
    ui->lineEdit_BATT_CUR->setText(QString("%1").arg(c) );
    ui->lineEdit_BATT_P->setText(QString("%1").arg(v*c/1000) );

    cpu_status->get_vvpn(&sync);

//    qDebug()<<"sync = "<< sync << "mV";
//    if(qAbs(sync) > 100){
        gettimeofday( &last_zero, NULL );
//        timer_sync->start();
//    }
}

void DebugSet::fresh_sync()
{
    //存储当前采样数据
    SYNC_DATA temp_data;
    cpu_status->get_vvpn(&temp_data.vcc);
    gettimeofday( &temp_data.t, NULL );
    sync_data.append(temp_data);

    int interval, offset, time_interval;

    //当存在6个有效数据时,进行过零点判断(过零点判断比较严格,必须单调)
    if(sync_data.length() > 6){
        if(sync_data.at(2).vcc < 0 && sync_data.at(3).vcc > 0
                && sync_data.at(0).vcc < sync_data.at(1).vcc && sync_data.at(1).vcc < sync_data.at(2).vcc
                && sync_data.at(3).vcc < sync_data.at(4).vcc && sync_data.at(4).vcc < sync_data.at(5).vcc){
            struct timeval zero_time = sync_data.at(5).t;       //6组数据的时间间隔
            time_interval = sync_data.at(5).t.tv_usec - sync_data.at(0).t.tv_usec;
            if(time_interval <0){
                time_interval += 1000000;
            }
//            qDebug()<<time_interval;
            zero_time.tv_usec -=  time_interval * sync_data.at(5).vcc / (sync_data.at(5).vcc - sync_data.at(0).vcc);
            if(zero_time.tv_usec < 0){
                zero_time.tv_sec -= 1;
                zero_time.tv_usec += 1000000;
            }
            interval = zero_time.tv_usec - last_zero.tv_usec;
            if(interval<0){
                interval += 1000000;
            }
            offset = interval % (1000000 / sqlcfg->get_para()->freq_val);
            if(offset > 10000){
                offset -= 1000000 / sqlcfg->get_para()->freq_val;
            }

            if(qAbs(offset) < 1000){
//                qDebug()<< "sync interval =" << interval << "\toffset = "<< offset /*<< "\trate = " << 100.0*offset/interval << "%"*/;
//                qDebug()<< "sync begin!!!!!!!";
                offset += 500;
                if(offset < 0){
                    offset += 1000000 / sqlcfg->get_para()->freq_val;
                }
                emit send_sync(100 * offset );
                timer_sync->stop();

            }
            last_zero = zero_time;
        }
        sync_data.removeFirst();
    }
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
        tab4->setStyleSheet("QLabel{border: 0px solid darkGray;}");
        tab5->setStyleSheet("QLabel{border: 0px solid darkGray;}");

        switch (key_val->grade.val3) {
        case 1:         //TEV
            ui->lineEdit_TEV1_line->deselect();
            ui->lineEdit_TEV1_ZERO->deselect();
            ui->lineEdit_TEV1_NOISE->deselect();
            ui->lineEdit_TEV2_line->deselect();
            ui->lineEdit_TEV2_ZERO->deselect();
            ui->lineEdit_TEV2_NOISE->deselect();
            switch (key_val->grade.val4) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab4->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab5->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            case 1:
                ui->lineEdit_TEV1_line->selectAll();
                break;
            case 2:
                ui->lineEdit_TEV1_ZERO->selectAll();
                break;
            case 3:
                ui->lineEdit_TEV1_NOISE->selectAll();
                break;
            case 4:
                ui->lineEdit_TEV2_line->selectAll();
                break;
            case 5:
                ui->lineEdit_TEV2_ZERO->selectAll();
                break;
            case 6:
                ui->lineEdit_TEV2_NOISE->selectAll();
                break;
            default:
                break;
            }
            break;
        case 2:         //HFCT
            ui->lineEdit_HFCT1_THRESHOLD->deselect();
            ui->lineEdit_HFCT1_ZERO->deselect();
            ui->lineEdit_HFCT2_THRESHOLD->deselect();
            ui->lineEdit_HFCT2_ZERO->deselect();
            switch (key_val->grade.val4) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab4->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab4->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab5->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            case 1:
                ui->lineEdit_HFCT1_THRESHOLD->selectAll();
                break;
            case 2:
                ui->lineEdit_HFCT1_ZERO->selectAll();
                break;
            case 3:
                ui->lineEdit_HFCT2_THRESHOLD->selectAll();
                break;
            case 4:
                ui->lineEdit_HFCT2_ZERO->selectAll();
                break;
            default:
                break;
            }
            break;
        case 3:         //AA
            ui->lineEdit_AA1_Step->deselect();
            ui->lineEdit_AA1_offset->deselect();
            ui->lineEdit_AA2_Step->deselect();
            ui->lineEdit_AA2_offset->deselect();
            switch (key_val->grade.val4) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab4->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab5->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            case 1:
                ui->lineEdit_AA1_Step->selectAll();
                break;
            case 2:
                ui->lineEdit_AA1_offset->selectAll();
                break;
            case 4:
                ui->lineEdit_AA2_Step->selectAll();
                break;
            case 5:
                ui->lineEdit_AA2_offset->selectAll();
                break;
            default:
                break;
            }
            break;
        case 4:         //AE
            ui->lineEdit_AE1_Step->deselect();
            ui->lineEdit_AE1_offset->deselect();
            ui->lineEdit_AE2_Step->deselect();
            ui->lineEdit_AE2_offset->deselect();
            switch (key_val->grade.val4) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab4->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab5->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            case 1:
                ui->lineEdit_AE1_Step->selectAll();
                break;
            case 2:
                ui->lineEdit_AE1_offset->selectAll();
                break;
            case 4:
                ui->lineEdit_AE2_Step->selectAll();
                break;
            case 5:
                ui->lineEdit_AE2_offset->selectAll();
                break;
            default:
                break;
            }
            break;
        case 5:         //通道
            ui->pbt_HC1->setChecked(false);
            ui->pbt_HC2->setChecked(false);
            ui->pbt_LC1->setChecked(false);
            ui->pbt_LC2->setChecked(false);
            ui->pbt_Double->setChecked(false);
            switch (key_val->grade.val4) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab4->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab5->setStyleSheet("QLabel{border: 0px solid darkGray;}");
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
            default:
                break;
            }
            break;
        case 6:         //状态监测
            tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab4->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab5->setStyleSheet("QLabel{border: 1px solid darkGray;}");
            break;
        default:
            break;
        }
    }
}




