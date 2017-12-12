#include "debugset.h"
#include "ui_debugui.h"
#include <QButtonGroup>
#include <QDir>
#include "../Common/common.h"
#include "IO/Com/rdb/rdb.h"

#include <sys/time.h>    // for gettimeofday()

#define SETTING_NUM             5           //设置菜单条目数
#define SETTING_NUM_TEV         6
#define SETTING_NUM_HFCT        4
#define SETTING_NUM_AA          2
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
    tab2 = new QLabel(tr("超声波"),ui->tabWidget->tabBar());
    Common::setTab(tab2);
    tab3 = new QLabel(tr("通道设置"),ui->tabWidget->tabBar());
    Common::setTab(tab3);
    tab4 = new QLabel(tr("硬件监测"),ui->tabWidget->tabBar());
    Common::setTab(tab4);

    ui->tabWidget->tabBar()->setTabButton(0,QTabBar::LeftSide,tab0);
    ui->tabWidget->tabBar()->setTabButton(1,QTabBar::LeftSide,tab1);
    ui->tabWidget->tabBar()->setTabButton(2,QTabBar::LeftSide,tab2);
    ui->tabWidget->tabBar()->setTabButton(3,QTabBar::LeftSide,tab3);
    ui->tabWidget->tabBar()->setTabButton(4,QTabBar::LeftSide,tab4);

    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        ui->tabWidget->widget(i)->setStyleSheet("QWidget {background-color:lightGray;}");
    }

    QButtonGroup *group1 = new QButtonGroup(this);
    QButtonGroup *group2 = new QButtonGroup(this);
    QButtonGroup *group3 = new QButtonGroup(this);
    QButtonGroup *group4 = new QButtonGroup(this);
    group1->addButton( ui->rb_HC1_TEV1 );
    group1->addButton( ui->rb_HC1_TEV2 );
    group1->addButton( ui->rb_HC1_HFCT1 );
    group1->addButton( ui->rb_HC1_HFCT2 );
    group1->addButton( ui->rb_HC1_UHF1 );
    group1->addButton( ui->rb_HC1_UHF2 );
    group1->addButton( ui->rb_HC1_Disable );
    group2->addButton( ui->rb_HC2_TEV1 );
    group2->addButton( ui->rb_HC2_TEV2 );
    group2->addButton( ui->rb_HC2_HFCT1 );
    group2->addButton( ui->rb_HC2_HFCT2 );
    group2->addButton( ui->rb_HC2_UHF1 );
    group2->addButton( ui->rb_HC2_UHF2 );
    group2->addButton( ui->rb_HC2_Disable );
    group3->addButton( ui->rb_LC1_AA );
    group3->addButton( ui->rb_LC1_Disable );
    group4->addButton( ui->rb_LC2_AE );
    group4->addButton( ui->rb_LC2_Disable );
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
    data->set_send_para (sp_tev1_zero, 0x8000 - sql_para.tev1_sql.fpga_zero);
    data->set_send_para (sp_tev1_threshold, sql_para.tev1_sql.fpga_threshold);
    data->set_send_para (sp_tev2_zero, 0x8000 - sql_para.tev2_sql.fpga_zero);
    data->set_send_para (sp_tev2_threshold, sql_para.tev2_sql.fpga_threshold);

    data->set_send_para (sp_hfct1_zero, 0x8000 - sql_para.hfct1_sql.fpga_zero);
    data->set_send_para (sp_hfct1_threshold, sql_para.hfct1_sql.fpga_threshold);
    data->set_send_para (sp_hfct2_zero, 0x8000 - sql_para.hfct2_sql.fpga_zero);
    data->set_send_para (sp_hfct2_threshold, sql_para.hfct2_sql.fpga_threshold);

    uint temp = sqlcfg->get_working_mode((MODE)sql_para.menu_h1, (MODE)sql_para.menu_h2);
    data->set_send_para (sp_working_mode, temp);
    qDebug()<<"current working mode code is: " << temp;
    qDebug()<<"debug para saved!";
    emit update_statusBar(tr("【调试模式】设置已保存！"));
    if(flag){
        QCoreApplication::quit();
    }
}

void DebugSet::reload()
{
    //TEV
    ui->lineEdit_TEV1_THRESHOLD->setText(QString("%1 mV").arg((int)Common::physical_value(sql_para.tev1_sql.fpga_threshold, TEV1) ) );
    ui->lineEdit_TEV1_ZERO->setText(QString("%1").arg(sql_para.tev1_sql.fpga_zero) );
    ui->lineEdit_TEV1_NOISE->setText(QString("%1").arg(sql_para.tev1_sql.tev_offset1) );

    ui->lineEdit_TEV2_THRESHOLD->setText(QString("%1 mV").arg((int)Common::physical_value(sql_para.tev2_sql.fpga_threshold, TEV2) ) );
    ui->lineEdit_TEV2_ZERO->setText(QString("%1").arg(sql_para.tev2_sql.fpga_zero) );
    ui->lineEdit_TEV2_NOISE->setText(QString("%1").arg(sql_para.tev2_sql.tev_offset1) );

    //HFCT
    ui->lineEdit_HFCT1_THRESHOLD->setText(QString("%1 mV").arg((int)Common::physical_value(sql_para.hfct1_sql.fpga_threshold, HFCT1) ) );
    ui->lineEdit_HFCT1_ZERO->setText(QString("%1").arg(sql_para.hfct1_sql.fpga_zero) );

    ui->lineEdit_HFCT2_THRESHOLD->setText(QString("%1 mV").arg((int)Common::physical_value(sql_para.hfct2_sql.fpga_threshold, HFCT2) ) );
    ui->lineEdit_HFCT2_ZERO->setText(QString("%1").arg(sql_para.hfct2_sql.fpga_zero) );

    //AA
    ui->lineEdit_AA_Step->setText(QString("%1").arg(sql_para.aaultra_sql.aa_step));
    ui->lineEdit_AA_offset->setText(QString("%1").arg(sql_para.aaultra_sql.aa_offset));

    //通道选择
    switch (sql_para.menu_h1) {
    case TEV1:
        ui->rb_HC1_TEV1->setChecked(true);
        break;
    case TEV2:
        ui->rb_HC1_TEV2->setChecked(true);
        break;
    case HFCT1:
        ui->rb_HC1_HFCT1->setChecked(true);
        break;
    case HFCT2:
        ui->rb_HC1_HFCT2->setChecked(true);
        break;
    case UHF1:
        ui->rb_HC1_UHF1->setChecked(true);
        break;
    case UHF2:
        ui->rb_HC1_UHF2->setChecked(true);
        break;
    case Disable:
        ui->rb_HC1_Disable->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para.menu_h2) {
    case TEV1:
        ui->rb_HC2_TEV1->setChecked(true);
        break;
    case TEV2:
        ui->rb_HC2_TEV2->setChecked(true);
        break;
    case HFCT1:
        ui->rb_HC2_HFCT1->setChecked(true);
        break;
    case HFCT2:
        ui->rb_HC2_HFCT2->setChecked(true);
        break;
    case UHF1:
        ui->rb_HC2_UHF1->setChecked(true);
        break;
    case UHF2:
        ui->rb_HC2_UHF2->setChecked(true);
        break;
    case Disable:
        ui->rb_HC2_Disable->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para.menu_l2) {
    case AE_Ultrasonic:
        ui->rb_LC2_AE->setChecked(true);
        break;
    case Disable:
        ui->rb_LC2_Disable->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para.menu_l1) {
    case AA_Ultrasonic:
        ui->rb_LC1_AA->setChecked(true);
        break;
    case Disable:
        ui->rb_LC1_Disable->setChecked(true);
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
        switch (key_val->grade.val3) {
        case 1:         //TEV
            switch (key_val->grade.val4) {
            case 1:
                sql_para.tev1_sql.fpga_threshold += Common::code_value(1,TEV1) * d;
                break;
            case 2:
                sql_para.tev1_sql.fpga_zero += d;
                break;
            case 3:
                sql_para.tev1_sql.tev_offset1 += d;
                break;
            case 4:
                sql_para.tev2_sql.fpga_threshold += Common::code_value(1,TEV2) * d;
                break;
            case 5:
                sql_para.tev2_sql.fpga_zero += d;
                break;
            case 6:
                sql_para.tev2_sql.tev_offset1 += d;
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
                Common::change_index(sql_para.aaultra_sql.aa_step, 0.5 * d, 10, 0.5);
                break;
            case 2:
                sql_para.aaultra_sql.aa_offset += d;
                break;
            default:
                break;
            }
            break;
        case 4:     //通道
            switch (key_val->grade.val4) {
            case 1:     //高频通道1
                Common::change_index(sql_para.menu_h1, d, UHF2, Disable);
                if(sql_para.menu_h1 == sql_para.menu_h2){       //如果冲突,再执行一次
                    Common::change_index(sql_para.menu_h1, d, UHF2, Disable);
                }
                break;
            case 2:
                Common::change_index(sql_para.menu_h2, d, UHF2, Disable);
                if(sql_para.menu_h2 == sql_para.menu_h1){
                    Common::change_index(sql_para.menu_h2, d, UHF2, Disable);
                }
                break;
            case 3:
                if(sql_para.menu_l1 == AA_Ultrasonic){
                    sql_para.menu_l1 = Disable;
                }
                else if(sql_para.menu_l1 == Disable && ui->rb_LC1_AA->isEnabled()){       //判断AA是否可用
                    sql_para.menu_l1 = AA_Ultrasonic;
                }

                break;
            case 4:
                if(sql_para.menu_l2 == AE_Ultrasonic){
                    sql_para.menu_l2 = Disable;
                }
                else if(sql_para.menu_l2 == Disable && ui->rb_LC2_AE->isEnabled()){       //判断AE是否可用
                    sql_para.menu_l2 = AE_Ultrasonic;
                }
                break;
            case 5:
                if(sql_para.menu_double == Double_Channel){
                    sql_para.menu_double = Disable;
                }
                else if(sql_para.menu_double == Disable && ui->rb_Double_Enable->isEnabled()){       //判断双通道是否可用
                    sql_para.menu_double = Double_Channel;
                }
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
        case 4:                     //通道
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

    yc_get_value(0,AA_biased_adv,1, &temp_data, b, a);
    ui->lab_offset_noise_aa->setText(QString("%1").arg(temp_data.f_val));
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

        switch (key_val->grade.val3) {
        case 1:         //TEV
            ui->lineEdit_TEV1_THRESHOLD->deselect();
            ui->lineEdit_TEV1_ZERO->deselect();
            ui->lineEdit_TEV1_NOISE->deselect();
            ui->lineEdit_TEV2_THRESHOLD->deselect();
            ui->lineEdit_TEV2_ZERO->deselect();
            ui->lineEdit_TEV2_NOISE->deselect();
            switch (key_val->grade.val4) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab4->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            case 1:
                ui->lineEdit_TEV1_THRESHOLD->selectAll();
                break;
            case 2:
                ui->lineEdit_TEV1_ZERO->selectAll();
                break;
            case 3:
                ui->lineEdit_TEV1_NOISE->selectAll();
                break;
            case 4:
                ui->lineEdit_TEV2_THRESHOLD->selectAll();
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
            ui->lineEdit_AA_Step->deselect();
            ui->lineEdit_AA_offset->deselect();
            switch (key_val->grade.val4) {
            case 0:
                tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab2->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                tab4->setStyleSheet("QLabel{border: 0px solid darkGray;}");
                break;
            case 1:
                ui->lineEdit_AA_Step->selectAll();
                break;
            case 2:
                ui->lineEdit_AA_offset->selectAll();
                break;
            default:
                break;
            }
            break;
        case 4:         //通道
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
                tab3->setStyleSheet("QLabel{border: 1px solid darkGray;}");
                tab4->setStyleSheet("QLabel{border: 0px solid darkGray;}");
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
        case 5:         //状态监测
            tab0->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab1->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab2->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab3->setStyleSheet("QLabel{border: 0px solid darkGray;}");
            tab4->setStyleSheet("QLabel{border: 1px solid darkGray;}");
            break;
        default:
            break;
        }
    }
}




