#include "debugset.h"
#include "ui_debugui.h"
#include <QButtonGroup>
#include <QDir>
#include "../Common/common.h"

#define SETTING_NUM             5           //设置菜单条目数
#define SETTING_NUM_TEV         6
#define SETTING_NUM_HFCT        4
#define SETTING_NUM_AA          2
#define SETTING_NUM_CHANNEL     5
#define SETTING_NUM_ADVANCED    1

DebugSet::DebugSet(G_PARA *g_data,QWidget *parent) : QFrame(parent),ui(new Ui::DebugUi)
{
    key_val = NULL;

    data = g_data;

    sql_para = *sqlcfg->get_para();

    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    ui->setupUi(this);

    ui->tabWidget->setStyleSheet("QTabBar::tab {border: 0px solid white; min-width: 0ex;padding: 1px; }"
                                 "QTabBar::tab:selected{ background:lightGray;  }"
                                 "QTabBar::tab:!selected{ background:transparent;   }"
                                 "QTabWidget::pane{border-width:0px;}"
                                 );
    QLabel *menu_icon0 = new QLabel(tr("地电波"),ui->tabWidget->tabBar());
    Common::setTab(menu_icon0);
    QLabel *menu_icon1 = new QLabel(tr("高频CT"),ui->tabWidget->tabBar());
    Common::setTab(menu_icon1);
    QLabel *menu_icon2 = new QLabel(tr("超声波"),ui->tabWidget->tabBar());
    Common::setTab(menu_icon2);
    QLabel *menu_icon3 = new QLabel(tr("通道设置"),ui->tabWidget->tabBar());
    Common::setTab(menu_icon3);
    QLabel *menu_icon4 = new QLabel(tr("高级"),ui->tabWidget->tabBar());
    Common::setTab(menu_icon4);

    ui->tabWidget->tabBar()->setTabButton(0,QTabBar::LeftSide,menu_icon0);
    ui->tabWidget->tabBar()->setTabButton(1,QTabBar::LeftSide,menu_icon1);
    ui->tabWidget->tabBar()->setTabButton(2,QTabBar::LeftSide,menu_icon2);
    ui->tabWidget->tabBar()->setTabButton(3,QTabBar::LeftSide,menu_icon3);
    ui->tabWidget->tabBar()->setTabButton(4,QTabBar::LeftSide,menu_icon4);

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
    group1->addButton( ui->rb_HC1_Disable );
    group2->addButton( ui->rb_HC2_TEV1 );
    group2->addButton( ui->rb_HC2_TEV2 );
    group2->addButton( ui->rb_HC2_HFCT1 );
    group2->addButton( ui->rb_HC2_HFCT2 );
    group2->addButton( ui->rb_HC2_Disable );
    group3->addButton( ui->rb_LC1_AA );
    group3->addButton( ui->rb_LC1_Disable );
    group4->addButton( ui->rb_LC2_AE );
    group4->addButton( ui->rb_LC2_Disable );


    iniUi();

    //密码部件
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

    this->hide();
}



void DebugSet::set_offset_suggest1(int a, int b)
{
    ui->lab_offset1->setText(QString("%1").arg(a));
    ui->lab_offset2->setText(QString("%1").arg(b));
}

void DebugSet::set_offset_suggest2(int a, int b)
{
    ui->lab_offset3->setText(QString("%1").arg(a));
    ui->lab_offset4->setText(QString("%1").arg(b));
}

void DebugSet::set_AA_offset_suggest(int a)
{
    ui->label_AA_offset->setText(QString("%1").arg(a));
}

void DebugSet::iniUi()
{
    ui->tabWidget->setCurrentIndex(0);
    readSql();
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
    if(sql_para.menu_aa == sqlcfg->get_para()->menu_aa && sql_para.menu_ae == sqlcfg->get_para()->menu_ae
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

    uint temp = sqlcfg->get_working_mode(sql_para.menu_h1, sql_para.menu_h2);
    data->set_send_para (sp_working_mode, temp);
    qDebug()<<"current working mode code is: " << temp;
    qDebug()<<"debug para saved!";
    emit update_statusBar(tr("【调试模式】设置已保存！"));
    if(flag){
        QCoreApplication::quit();
    }
}

void DebugSet::readSql()
{
    //TEV
    ui->lineEdit_TEV1_THRESHOLD->setText(QString("%1").arg(sql_para.tev1_sql.fpga_threshold) );
    ui->lineEdit_TEV1_ZERO->setText(QString("%1").arg(sql_para.tev1_sql.fpga_zero) );
    ui->lineEdit_TEV1_NOISE->setText(QString("%1").arg(sql_para.tev1_sql.tev_offset1) );

    ui->lineEdit_TEV2_THRESHOLD->setText(QString("%1").arg(sql_para.tev2_sql.fpga_threshold) );
    ui->lineEdit_TEV2_ZERO->setText(QString("%1").arg(sql_para.tev2_sql.fpga_zero) );
    ui->lineEdit_TEV2_NOISE->setText(QString("%1").arg(sql_para.tev2_sql.tev_offset1) );

    //AA
    ui->lineEdit_AA_Step->setText(QString("%1").arg(sql_para.aaultra_sql.aa_step));
    ui->lineEdit_AA_offset->setText(QString("%1").arg(sql_para.aaultra_sql.aa_offset));

    //高级
    ui->lineEdit_MaxRecNum->setText(QString("%1").arg(sql_para.max_rec_num));

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
    case Disable:
        ui->rb_HC2_Disable->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para.menu_ae) {
    case AE_Ultrasonic:
        ui->rb_LC2_AE->setChecked(true);
        break;
    case Disable:
        ui->rb_LC2_Disable->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para.menu_aa) {
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

                iniUi();
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
            if(key_val->grade.val4 == 0){   //判断在二级菜单
                if(key_val->grade.val3>1){
                    key_val->grade.val3 --;
                }
                else{
                    key_val->grade.val3 = SETTING_NUM;
                }
                iniUi();        //刷新列表
            }
            else{                           //判断在三级菜单
                switch (key_val->grade.val3) {
                case 1:                 //TEV
                    if(key_val->grade.val4 >1 ){
                        key_val->grade.val4 --;
                    }
                    else{
                        key_val->grade.val4 = SETTING_NUM_TEV;
                    }
                    break;
                case 2:                 //HFCT
                    if(key_val->grade.val4 >1 ){
                        key_val->grade.val4 --;
                    }
                    else{
                        key_val->grade.val4 = SETTING_NUM_HFCT;
                    }
                    break;
                case 3:                 //AA
                    if(key_val->grade.val4 >1 ){
                        key_val->grade.val4 --;
                    }
                    else{
                        key_val->grade.val4 = SETTING_NUM_AA;
                    }
                    break;
                case 4:                 //通道设置
                    if(key_val->grade.val4 >1 ){
                        key_val->grade.val4 --;
                    }
                    else{
                        key_val->grade.val4 = SETTING_NUM_CHANNEL;
                    }
                    break;
                case 5:                 //高级
                    if(key_val->grade.val4 >1){
                        key_val->grade.val4 --;
                    }
                    else{
                        key_val->grade.val4 = SETTING_NUM_ADVANCED;
                    }
                    break;
                default:
                    break;
                }
            }
        }
        else{
            password.append("U");
            passwordEdit->setText(passwordEdit->text()+"*");
        }
        break;
    case KEY_DOWN:
        if(pass){
            if(key_val->grade.val4 == 0){   //判断在二级菜单
                if(key_val->grade.val3 < SETTING_NUM){

                    key_val->grade.val3 ++;
                }
                else{
                    key_val->grade.val3 = 1;
                }
                iniUi();
            }
            else{                           //判断在三级菜单
                switch (key_val->grade.val3) {
                case 1:                     //TEV
                    if(key_val->grade.val4 < SETTING_NUM_TEV){
                        key_val->grade.val4 ++;
                    }
                    else{
                        key_val->grade.val4 = 1;
                    }
                    break;
                case 2:                     //HFCT
                    if(key_val->grade.val4 < SETTING_NUM_HFCT){
                        key_val->grade.val4 ++;
                    }
                    else{
                        key_val->grade.val4 = 1;
                    }
                    break;
                case 3:                     //AA
                    if(key_val->grade.val4 < SETTING_NUM_AA){
                        key_val->grade.val4 ++;
                    }
                    else{
                        key_val->grade.val4 = 1;
                    }
                    break;
                case 4:                     //通道
                    if(key_val->grade.val4 < SETTING_NUM_CHANNEL){
                        key_val->grade.val4 ++;
                    }
                    else{
                        key_val->grade.val4 = 1;
                    }
                    break;
                case 5:                     //高级
                    if(key_val->grade.val4 < SETTING_NUM_ADVANCED){
                        key_val->grade.val4 ++;
                    }
                    else{
                        key_val->grade.val4 = 1;
                    }
                    break;
                default:
                    break;
                }
            }
        }
        else{
            password.append("D");
            passwordEdit->setText(passwordEdit->text()+"*");
        }
        break;
    case KEY_LEFT:
        if(pass){
            if(key_val->grade.val4 == 0){
            }
            else{
                switch (key_val->grade.val3) {
                case 1:         //TEV
                    switch (key_val->grade.val4) {
                    case 1:
                        sql_para.tev1_sql.fpga_threshold--;
                        break;
                    case 2:
                        sql_para.tev1_sql.fpga_zero--;
                        break;
                    case 3:
                        sql_para.tev1_sql.tev_offset1--;
                        break;
                    case 4:
                        sql_para.tev2_sql.fpga_threshold--;
                        break;
                    case 5:
                        sql_para.tev2_sql.fpga_zero--;
                        break;
                    case 6:
                        sql_para.tev2_sql.tev_offset1--;
                        break;
                    default:
                        break;
                    }
                    break;
                case 2:         //HFCT
                    switch (key_val->grade.val4) {
                    case 1:
                        sql_para.hfct1_sql.fpga_threshold--;
                        break;
                    case 2:
                        sql_para.hfct1_sql.fpga_zero--;
                        break;
                    case 3:
                        sql_para.hfct2_sql.fpga_threshold--;
                        break;
                    case 4:
                        sql_para.hfct2_sql.fpga_zero--;
                        break;
                    default:
                        break;
                    }
                    break;
                case 3:     //AA
                    if(key_val->grade.val4 == 1){
                        if(sql_para.aaultra_sql.aa_step > 0.25)
                            sql_para.aaultra_sql.aa_step -= 0.5;
                    }
                    else if(key_val->grade.val4 == 2){
                        sql_para.aaultra_sql.aa_offset --;
                    }
                    break;
                case 4:     //通道
                    switch (key_val->grade.val4) {
                    case 1:
                        switch (sql_para.menu_h1) {
                        case TEV1:
                            sql_para.menu_h1 = Disable;
                            break;
                        case TEV2:
                            if(sql_para.menu_h2 == TEV1){
                                sql_para.menu_h1 = Disable;
                            }
                            else{
                                sql_para.menu_h1 = TEV1;
                            }
                            break;
                        case HFCT1:
                            if(sql_para.menu_h2 == TEV2){
                                sql_para.menu_h1 = TEV1;
                            }
                            else{
                                sql_para.menu_h1 = TEV2;
                            }
                            break;
                        case HFCT2:
                            if(sql_para.menu_h2 == HFCT1){
                                sql_para.menu_h1 = TEV2;
                            }
                            else{
                                sql_para.menu_h1 = HFCT1;
                            }
                            break;
                        case Disable:
                            if(sql_para.menu_h2 == HFCT2){
                                sql_para.menu_h1 = HFCT1;
                            }
                            else{
                                sql_para.menu_h1 = HFCT2;
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    case 2:
                        switch (sql_para.menu_h2) {
                        case TEV1:
                            sql_para.menu_h2 = Disable;
                            break;
                        case TEV2:
                            if(sql_para.menu_h1 == TEV1){
                                sql_para.menu_h2 = Disable;
                            }
                            else{
                                sql_para.menu_h2 = TEV1;
                            }
                            break;
                        case HFCT1:
                            if(sql_para.menu_h1 == TEV2){
                                sql_para.menu_h2 = TEV1;
                            }
                            else{
                                sql_para.menu_h2 = TEV2;
                            }
                            break;
                        case HFCT2:
                            if(sql_para.menu_h1 == HFCT1){
                                sql_para.menu_h2 = TEV2;
                            }
                            else{
                                sql_para.menu_h2 = HFCT1;
                            }
                            break;
                        case Disable:
                            if(sql_para.menu_h1 == HFCT2){
                                sql_para.menu_h2 = HFCT1;
                            }
                            else{
                                sql_para.menu_h2 = HFCT2;
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    case 3:
                        if(sql_para.menu_aa == AA_Ultrasonic){
                            sql_para.menu_aa = Disable;
                        }
                        else if(sql_para.menu_aa == Disable && ui->rb_LC1_AA->isEnabled()){       //判断AA是否可用
                            sql_para.menu_aa = AA_Ultrasonic;
                        }

                        break;
                    case 4:
                        if(sql_para.menu_ae == AE_Ultrasonic){
                            sql_para.menu_ae = Disable;
                        }
                        else if(sql_para.menu_ae == Disable && ui->rb_LC2_AE->isEnabled()){       //判断AE是否可用
                            sql_para.menu_ae = AE_Ultrasonic;
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
                case 5:     //高级
                    if(sql_para.max_rec_num >20)
                        sql_para.max_rec_num -= 10;
                    break;
                default:
                    break;
                }
            }
        }
        else{
            password.append("L");
            passwordEdit->setText(passwordEdit->text()+"*");
        }

        break;
    case KEY_RIGHT:
        if(pass){
            if(key_val->grade.val4 == 0 && key_val->grade.val3 != 0){   //必须第三层处于工作状态
                key_val->grade.val4 =1;
            }
            else{
                switch (key_val->grade.val3) {
                case 1:     //TEV
                    switch (key_val->grade.val4) {
                    case 1:
                        sql_para.tev1_sql.fpga_threshold++;
                        break;
                    case 2:
                        sql_para.tev1_sql.fpga_zero++;
                        break;
                    case 3:
                        sql_para.tev1_sql.tev_offset1++;
                        break;
                    case 4:
                        sql_para.tev2_sql.fpga_threshold++;
                        break;
                    case 5:
                        sql_para.tev2_sql.fpga_zero++;
                        break;
                    case 6:
                        sql_para.tev2_sql.tev_offset1++;
                        break;
                    default:
                        break;
                    }
                    break;
                case 2:     //HFCT
                    switch (key_val->grade.val4) {
                    case 1:
                        sql_para.hfct1_sql.fpga_threshold++;
                        break;
                    case 2:
                        sql_para.hfct1_sql.fpga_zero++;
                        break;
                    case 3:
                        sql_para.hfct2_sql.fpga_threshold++;
                        break;
                    case 4:
                        sql_para.hfct2_sql.fpga_zero++;
                        break;
                    default:
                        break;
                    }
                    break;
                case 3:     //AA
                    if(key_val->grade.val4 == 1){
                        sql_para.aaultra_sql.aa_step += 0.5;
                    }
                    else if(key_val->grade.val4 == 2){
                        sql_para.aaultra_sql.aa_offset ++;
                    }
                    break;
                case 4:     //通道
                    switch (key_val->grade.val4) {
                    case 1:
                        switch (sql_para.menu_h1) {
                        case TEV1:
                            if(sql_para.menu_h2 == TEV2){
                                sql_para.menu_h1 = HFCT1;
                            }
                            else{
                                sql_para.menu_h1 = TEV2;
                            }
                            break;
                        case TEV2:
                            if(sql_para.menu_h2 == HFCT1){
                                sql_para.menu_h1 = HFCT2;
                            }
                            else{
                                sql_para.menu_h1 = HFCT1;
                            }
                            break;
                        case HFCT1:
                            if(sql_para.menu_h2 == HFCT2){
                                sql_para.menu_h1 = Disable;
                            }
                            else{
                                sql_para.menu_h1 = HFCT2;
                            }
                            break;
                        case HFCT2:
                            sql_para.menu_h1 = Disable;
                            break;
                        case Disable:
                            if(sql_para.menu_h2 == TEV1){
                                sql_para.menu_h1 = TEV2;
                            }
                            else{
                                sql_para.menu_h1 = TEV1;
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    case 2:
                        switch (sql_para.menu_h2) {
                        case TEV1:
                            if(sql_para.menu_h1 == TEV2){
                                sql_para.menu_h2 = HFCT1;
                            }
                            else{
                                sql_para.menu_h2 = TEV2;
                            }
                            break;
                        case TEV2:
                            if(sql_para.menu_h1 == HFCT1){
                                sql_para.menu_h2 = HFCT2;
                            }
                            else{
                                sql_para.menu_h2 = HFCT1;
                            }
                            break;
                        case HFCT1:
                            if(sql_para.menu_h1 == HFCT2){
                                sql_para.menu_h2 = Disable;
                            }
                            else{
                                sql_para.menu_h2 = HFCT2;
                            }
                            break;
                        case HFCT2:
                            sql_para.menu_h2 = Disable;
                            break;
                        case Disable:
                            if(sql_para.menu_h1 == TEV1){
                                sql_para.menu_h2 = TEV2;
                            }
                            else{
                                sql_para.menu_h2 = TEV1;
                            }
                            break;
                        default:
                            break;
                        }
                        break;
                    case 3:
                        if(sql_para.menu_aa == AA_Ultrasonic){
                            sql_para.menu_aa = Disable;
                        }
                        else if(sql_para.menu_aa == Disable && ui->rb_LC1_AA->isEnabled()){       //判断AA是否可用
                            sql_para.menu_aa = AA_Ultrasonic;
                        }

                        break;
                    case 4:
                        if(sql_para.menu_ae == AE_Ultrasonic){
                            sql_para.menu_ae = Disable;
                        }
                        else if(sql_para.menu_ae == Disable && ui->rb_LC2_AE->isEnabled()){       //判断AE是否可用
                            sql_para.menu_ae = AE_Ultrasonic;
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
                case 5:     //高级
                    if(key_val->grade.val4 == 3){
                        sql_para.max_rec_num += 10;
                    }
                    break;
                default:
                    break;
                }
            }
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

void DebugSet::fresh()
{
    //    printf("\nkey_val->grade.val2 is : %d",key_val->grade.val2);
    //    printf("\tkey_val->grade.val3 is : %d",key_val->grade.val3);
    //    printf("\tkey_val->grade.val3 is : %d \n",key_val->grade.val3);

    if(pass){
        if(key_val->grade.val3){
            ui->tabWidget->setCurrentIndex(key_val->grade.val3-1);
        }

        //刷新数据
        readSql();

        //刷新组件状态
        switch (key_val->grade.val3) {
        case 1:         //TEV
            ui->lineEdit_TEV1_THRESHOLD->deselect();
            ui->lineEdit_TEV1_ZERO->deselect();
            ui->lineEdit_TEV1_NOISE->deselect();
            ui->lineEdit_TEV2_THRESHOLD->deselect();
            ui->lineEdit_TEV2_ZERO->deselect();
            ui->lineEdit_TEV2_NOISE->deselect();
            switch (key_val->grade.val4) {
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
            if(key_val->grade.val4 == 0){
                ui->lineEdit_AA_Step->deselect();
                ui->lineEdit_AA_offset->deselect();
            }
            else if(key_val->grade.val4 == 1){
                ui->lineEdit_AA_Step->selectAll();
                ui->lineEdit_AA_offset->deselect();
            }
            else if(key_val->grade.val4 == 2){
                ui->lineEdit_AA_Step->deselect();
                ui->lineEdit_AA_offset->selectAll();
            }
            break;
        case 4:         //通道
//            ui->pbt_HC1->setStyleSheet("QPushButton {color:black;}");
            ui->pbt_HC1->setChecked(false);
//            ui->pbt_HC2->setStyleSheet("QPushButton {color:black;}");
            ui->pbt_HC2->setChecked(false);
//            ui->pbt_LC1->setStyleSheet("QPushButton {color:black;}");
            ui->pbt_LC1->setChecked(false);
//            ui->pbt_LC2->setStyleSheet("QPushButton {color:black;}");
            ui->pbt_LC2->setChecked(false);
//            ui->pbt_Double->setStyleSheet("QPushButton {color:black;}");
            ui->pbt_Double->setChecked(false);
            switch (key_val->grade.val4) {
            case 1:
//                ui->pbt_HC1->setStyleSheet("QPushButton {color:white;}");
                ui->pbt_HC1->setChecked(true);
                break;
            case 2:
//                ui->pbt_HC2->setStyleSheet("QPushButton {color:white;}");
                ui->pbt_HC2->setChecked(true);
                break;
            case 3:
//                ui->pbt_LC1->setStyleSheet("QPushButton {color:white;}");
                ui->pbt_LC1->setChecked(true);
                break;
            case 4:
//                ui->pbt_LC2->setStyleSheet("QPushButton {color:white;}");
                ui->pbt_LC2->setChecked(true);
                break;
            case 5:
//                ui->pbt_Double->setStyleSheet("QPushButton {color:white;}");
                ui->pbt_Double->setChecked(true);
                break;
            default:
                break;
            }
            break;
        case 5:         //高级
            ui->lineEdit_MaxRecNum->selectAll();
            break;
        default:
            break;
        }
    }
}




