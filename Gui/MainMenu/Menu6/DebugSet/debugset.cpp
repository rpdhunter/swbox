#include "debugset.h"
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
//#include <QPoint>
#include "ui_debugui.h"
#include <QDir>



DebugSet::DebugSet(QWidget *parent,G_PARA *g_data) : QFrame(parent),ui(new Ui::DebugUi)
{
    key_val = NULL;

    data = g_data;

    sql_para = sqlcfg->get_para();

    this->resize(455, 185);
    this->move(2, 31);
    ui->setupUi(this);
    ui->tabWidget->setStyleSheet("QTabWidget {background-color:lightGray;}");
    ui->tabWidget->widget(0)->setStyleSheet("QWidget {background-color:lightGray;}");
    ui->tabWidget->widget(1)->setStyleSheet("QWidget {background-color:lightGray;}");
    ui->tabWidget->widget(2)->setStyleSheet("QWidget {background-color:lightGray;}");

    ui->tabWidget->tabBar()->setStyle(new CustomTabStyle);
    iniUi();

    widget = new QWidget(this);
    widget->setStyleSheet("QWidget {background-color:lightGray;}");
    widget->resize(455, 185);
    widget->move(2, 31);


    lab1 = new QLabel(widget);
    lab1->setText(tr("请输入管理员密码，解锁调试模式"));
    lab2 = new QLabel(widget);
    //    lab2->move(20,70);
    passwordEdit = new QLineEdit(widget);
    //    passwordEdit->move(20,40);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(lab1);
    layout->addWidget(passwordEdit);
    layout->addWidget(lab2);
    widget->setLayout(layout);

    password_set = "UDLR";  //默认密码

    resetPassword();

    recWaveForm = new RecWaveForm(this);
    recWaveForm->hide();
    connect(this, SIGNAL(send_key(quint8)), recWaveForm, SLOT(trans_key(quint8)));

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

    //录波数据列表初始化
    QDir dir = QDir("/root/WaveForm/");
    int r = ui->listWidget->currentRow();
    ui->listWidget->clear();

    QStringList filters;
    filters << "*.DAT" ;
    dir.setNameFilters(filters);
    QStringList list = dir.entryList(QDir::Files);
    list.replaceInStrings(".DAT", "");
    ui->listWidget->addItems(list);

//    dir.setPath("/mmc/sdcard/WaveForm/");
//    list = dir.entryList(QDir::Files);

//    list.replaceInStrings(".DAT", "(SDCard)");

//    ui->listWidget->addItems(list);
    ui->listWidget->setCurrentRow(r);       //保存选择状态

}

void DebugSet::resetPassword()
{
    pass = false;
    password.clear();
    passwordEdit->clear();
    lab2->clear();
    this->widget->show();
    ui->tabWidget->hide();
}

void DebugSet::saveSql()
{
    sqlcfg->sql_save(sql_para);

    //这些设置中，只有这两个量是需要写入FPGA才能生效的
    data->send_para.tev1_zero.flag = true;
    data->send_para.tev1_zero.rval = ( 0x8000 - sql_para->tev1_sql.fpga_zero );
    data->send_para.tev1_threshold.flag = true;
    data->send_para.tev1_threshold.rval = sql_para->tev1_sql.fpga_threshold;

    qDebug()<<"debug para saved!";
    emit update_statusBar(tr("【调试模式】设置已保存！"));
}

void DebugSet::readSql()
{
    //TEV
    ui->lineEdit_TEV1_THRESHOLD->setText(QString("%1").arg(sql_para->tev1_sql.fpga_threshold) );
    ui->lineEdit_TEV1_ZERO->setText(QString("%1").arg(sql_para->tev1_sql.fpga_zero) );
    ui->lineEdit_TEV1_NOISE->setText(QString("%1").arg(sql_para->tev1_sql.tev_offset1) );

    ui->lineEdit_TEV2_THRESHOLD->setText(QString("%1").arg(sql_para->tev2_sql.fpga_threshold) );
    ui->lineEdit_TEV2_ZERO->setText(QString("%1").arg(sql_para->tev2_sql.fpga_zero) );
    ui->lineEdit_TEV2_NOISE->setText(QString("%1").arg(sql_para->tev2_sql.tev_offset1) );

    //AA
    ui->lineEdit_AA_Step->setText(QString("%1").arg(sql_para->aaultra_sql.aa_step));
    ui->lineEdit_AA_offset->setText(QString("%1").arg(sql_para->aaultra_sql.aa_offset));

    //高级
    ui->lineEdit_MaxRecNum->setText(QString("%1").arg(sql_para->max_rec_num));
    ui->lineEdit_MaxRecNum->setText(QString("%1").arg(sql_para->aaultra_sql.time));
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
            if(key_val->grade.val3 == 3 && (key_val->grade.val4 == 1 || key_val->grade.val4 == 2) ){   //录波
                if(ui->lab_recWv->text() != tr("正在录波，请耐心等待")){
                    emit startRecWv(ui->comboBox->currentIndex(),ui->lineEdit_time->text().toInt());
                    ui->lab_recWv->setText(tr("正在录波，请耐心等待"));
                }
//                saveSql();
            }
            else if(key_val->grade.val3 == 4 && key_val->grade.val4 != 0){  //查看波形
//                qDebug()<<"AAAA!" << ui->listWidget->currentRow();
                key_val->grade.val5 = 1;
                recWaveForm->working(key_val,ui->listWidget->currentItem()->text());
            }
            else{                                                   //保存
                saveSql();                
            }
        }
        else{
            //检查密码是否匹配
            if(password == password_set){
                qDebug()<<"password accepted!";
                this->widget->hide();
                ui->tabWidget->show();
                pass = true;

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

            resetPassword();

            fresh_parent();
        }
        else{
            key_val->grade.val4 = 0;    //退出三级菜单
            ui->listWidget->setCurrentRow(-1);
        }

        break;
    case KEY_UP:
        if(pass){
            if(key_val->grade.val4 == 0){   //判断在二级菜单
                if(key_val->grade.val3>1){
                    key_val->grade.val3 --;
                }
                else{
                    key_val->grade.val3 = 4;
                }
                iniUi();        //刷新列表
            }
            else{                           //判断在三级菜单
                switch (key_val->grade.val3) {
                case 0:

                    break;
                case 1:                 //TEV
                    if(key_val->grade.val4 >1 ){
                        key_val->grade.val4 --;
                    }
                    else{
                        key_val->grade.val4 = 6;
                    }
                    break;
                case 2:                 //AA
                    if(key_val->grade.val4 >1 ){
                        key_val->grade.val4 --;
                    }
                    else{
                        key_val->grade.val4 = 2;
                    }
                    break;
                case 3:                 //高级
                    if(key_val->grade.val4 >1){
                        key_val->grade.val4 --;
                    }
                    else{
                        key_val->grade.val4 = 3;
                    }

                    break;
                case 4:                 //录波列表
                    if(ui->listWidget->currentRow() != 0){
                        ui->listWidget->setCurrentRow(ui->listWidget->currentRow()-1);
                    }
                    else{
                        ui->listWidget->setCurrentRow(ui->listWidget->count()-1);
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
                if(key_val->grade.val3<4){

                    key_val->grade.val3 ++;
                }
                else{
                    key_val->grade.val3 = 1;
                }
                iniUi();
            }
            else{                           //判断在三级菜单
                switch (key_val->grade.val3) {
                case 0:

                    break;
                case 1:                     //TEV
                    if(key_val->grade.val4 <6){
                        key_val->grade.val4 ++;
                    }
                    else{
                        key_val->grade.val4 = 1;
                    }
                    break;
                case 2:                     //AA
                    if(key_val->grade.val4 <2){
                        key_val->grade.val4 ++;
                    }
                    else{
                        key_val->grade.val4 = 1;
                    }
                    break;
                case 3:                     //高级
                    if(key_val->grade.val4 < 3){
                        key_val->grade.val4 ++;
                    }
                    else{
                        key_val->grade.val4 = 1;
                    }
                    break;
                case 4:                     //列表
                    if(ui->listWidget->currentRow()<ui->listWidget->count()-1){
                        ui->listWidget->setCurrentRow(ui->listWidget->currentRow()+1);
                    }
                    else{
                        ui->listWidget->setCurrentRow(0);
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
                case 0:

                    break;
                case 1:         //TEV
                    switch (key_val->grade.val4) {
                    case 1:
                        sql_para->tev1_sql.fpga_threshold--;
                        break;
                    case 2:
                        sql_para->tev1_sql.fpga_zero--;
                        break;
                    case 3:
                        sql_para->tev1_sql.tev_offset1--;
                        break;
                    case 4:
                        sql_para->tev2_sql.fpga_threshold--;
                        break;
                    case 5:
                        sql_para->tev2_sql.fpga_zero--;
                        break;
                    case 6:
                        sql_para->tev2_sql.tev_offset1--;
                        break;
                    default:
                        break;
                    }
                    break;
                case 2:     //AA
                    if(key_val->grade.val4 == 1){
                        if(sql_para->aaultra_sql.aa_step > 0.25)
                            sql_para->aaultra_sql.aa_step -= 0.5;
                    }
                    else if(key_val->grade.val4 == 2){
                        sql_para->aaultra_sql.aa_offset --;
                    }
                    break;
                case 3:     //高级
                    if(key_val->grade.val4 == 1){
                        int index = ui->comboBox->currentIndex();
                        if(index>0){
                            ui->comboBox->setCurrentIndex(index-1);
                        }
                        else{
                            ui->comboBox->setCurrentIndex(ui->comboBox->count()-1);
                        }
                    }
                    else if(key_val->grade.val4 == 2){
                        if(ui->lineEdit_time->isEnabled()){
                            int v = ui->lineEdit_time->text().toInt();
                            if(v>1){
                                ui->lineEdit_time->setText(QString("%1").arg(v-1));
                            }
                        }
                    }
                    else if(key_val->grade.val4 == 3){
                        if(sql_para->max_rec_num >20)
                            sql_para->max_rec_num -= 10;
                    }

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
                if(key_val->grade.val3 == 4){
                    ui->listWidget->setCurrentRow(0);
                }
            }
            else{
                switch (key_val->grade.val3) {
                case 0:

                    break;
                case 1:     //TEV
                    switch (key_val->grade.val4) {
                    case 1:
                        sql_para->tev1_sql.fpga_threshold++;
                        break;
                    case 2:
                        sql_para->tev1_sql.fpga_zero++;
                        break;
                    case 3:
                        sql_para->tev1_sql.tev_offset1++;
                        break;
                    case 4:
                        sql_para->tev2_sql.fpga_threshold++;
                        break;
                    case 5:
                        sql_para->tev2_sql.fpga_zero++;
                        break;
                    case 6:
                        sql_para->tev2_sql.tev_offset1++;
                        break;
                    default:
                        break;
                    }
                    break;
                case 2:     //AA
                    if(key_val->grade.val4 == 1){
                        sql_para->aaultra_sql.aa_step += 0.5;
                    }
                    else if(key_val->grade.val4 == 2){
                        sql_para->aaultra_sql.aa_offset ++;
                    }
                    break;
                case 3:     //高级
                    if(key_val->grade.val4 == 1){
                        int index = ui->comboBox->currentIndex();
                        if(index<ui->comboBox->count()-1){
                            ui->comboBox->setCurrentIndex(index+1);
                        }
                        else{
                            ui->comboBox->setCurrentIndex(0);
                        }
                    }
                    else if(key_val->grade.val4 == 2){
                        if(ui->lineEdit_time->isEnabled()){
                            int v = ui->lineEdit_time->text().toInt();
                            if(v<60){
                                ui->lineEdit_time->setText(QString("%1").arg(v+1));
                            }
                        }
                    }
                    else if(key_val->grade.val4 == 3){
                        sql_para->max_rec_num += 10;
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

//处理显示录波信号
void DebugSet::showWaveData(VectorList wv,MODE mod)
{
    switch (mod) {
    case 0:     //TEV
        ui->lab_recWv->setText(tr("接到录波信号,通道%1").arg("TEV"));
        break;
    case 1:     //AA超声
        ui->lab_recWv->setText(tr("接到录波信号,通道%1").arg("AAUltrasonic"));
        break;
    case 2:

        break;
    case 3:

        break;
    default:
        break;
    }

//    wave.clear();
//    for(int i=0;i<wv.length();i++){
//        wave.append(QPoint(i,wv.at(i)));
//    }

//    curve->setSamples(wave);
//    plot->show();

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
        case 2:         //AA
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
        case 3:         //高级
            if(key_val->grade.val4 == 0){
                ui->comboBox->setStyleSheet("QComboBox { background: lightGray }");
                ui->lineEdit_time->deselect();
                ui->lineEdit_MaxRecNum->deselect();
            }
            else if(key_val->grade.val4 == 1){
                ui->comboBox->setStyleSheet("QComboBox { background: gray }");
                ui->lineEdit_time->deselect();
                ui->lineEdit_MaxRecNum->deselect();
            }
            else if(key_val->grade.val4 == 2){
                ui->comboBox->setStyleSheet("QComboBox { background: lightGray }");
                ui->lineEdit_time->selectAll();
                ui->lineEdit_MaxRecNum->deselect();
            }
            else if(key_val->grade.val4 == 3){
                ui->comboBox->setStyleSheet("QComboBox { background: lightGray }");
                ui->lineEdit_time->deselect();
                ui->lineEdit_MaxRecNum->selectAll();
            }
            break;
        default:
            break;
        }


    }

}


void DebugSet::on_comboBox_currentIndexChanged(int index)
{
    if(index == 2) {        //超声录波，时长可调
        ui->lineEdit_time->setEnabled(true);
    }
    else{
        ui->lineEdit_time->setEnabled(false);
    }
}
