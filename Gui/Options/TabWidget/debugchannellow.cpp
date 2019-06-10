#include "debugchannellow.h"
#include "ui_debugchannellow.h"
#include "Gui/Common/common.h"

DebugChannelLow::DebugChannelLow(SQL_PARA *sql, QWidget *parent) :
    TabWidget(sql,parent),
    ui(new Ui::DebugChannelLow)
{
    ui->setupUi(this);
    this->resize(TABWIDGET_X,TABWIDGET_Y);

    if(sql_para->menu_l1 == AA1){         //AA1模式隐藏传感器频率
        ui->lab_freq_L1->hide();
        ui->lineEdit_L1_Freq->hide();
//        ui->spacer_freq_L1->h
    }
    else{                                 //AE1模式隐藏摄像头
        ui->lab_camera_L1->hide();
        ui->checkBox_camera_L1->hide();
//        ui->layout_camera_L1->widget()->hide();
    }
    if(sql_para->menu_l2 == AE2){         //AE2模式隐藏摄像头
        ui->lab_camera_L2->hide();
        ui->checkBox_camera_L2->hide();
//        ui->layout_camera_L2->widget()->hide();
    }
    else{                                 //AA2模式隐藏传感器频率
        ui->lab_freq_L2->hide();
        ui->lineEdit_L2_Freq->hide();
//        ui->layout_freq_L2->widget()->hide();
    }

    switch (sql_para->menu_l1) {
    case AA1:
        sql_c1 = &sql_para->aa1_sql;
        break;
    case AE1:
        sql_c1 = &sql_para->ae1_sql;
        break;
    default:
        sql_c1 = NULL;
    }

    switch (sql_para->menu_l2) {
    case AA2:
        sql_c2 = &sql_para->aa2_sql;
        break;
    case AE2:
        sql_c2 = &sql_para->ae2_sql;
        break;
    default:
        sql_c2 = NULL;
    }

    QString style = "QLabel {font-family:WenQuanYi Micro Hei;font: bold; font-size:16px;color:green}";
    if(sql_c1 != NULL){
        ui->label_L1->setStyleSheet(style);
        ui->label_L1->setText(Common::mode_to_string((MODE)sql_para->menu_l1));

    }
    if(sql_c2 != NULL){
        ui->label_L2->setStyleSheet(style);
        ui->label_L2->setText(Common::mode_to_string((MODE)sql_para->menu_l2));
    }

    QButtonGroup *group1 = new QButtonGroup(this);
    QButtonGroup *group2 = new QButtonGroup(this);
    group1->addButton( ui->rbt_L1_envelope );
    group1->addButton( ui->rbt_L1_original );
    group2->addButton( ui->rbt_L2_envelope );
    group2->addButton( ui->rbt_L2_original );

    startTimer(1000);
}

DebugChannelLow::~DebugChannelLow()
{
    delete ui;
}

void DebugChannelLow::work()
{
    if(sql_c1 == NULL && sql_c2 == NULL)
        return;

    if(sql_c1 == NULL){
        row = 4;
    }
    else{
        row = 1;
    }
    fresh();
}

void DebugChannelLow::do_key_ok()
{
    fresh();
    emit save();
}

void DebugChannelLow::do_key_cancel()
{
    row = 0;
    fresh();
    emit quit();
}

void DebugChannelLow::do_key_up_down(int d)
{
    if(sql_c1 == NULL && sql_c2 == NULL)
        return;

    if(sql_c1 == NULL){
        Common::change_index(row, d, 6, 4);
    }
    else if(sql_c2 == NULL){
        Common::change_index(row, d, 3, 1);
    }
    else{
        Common::change_index(row, d, 6, 1);
    }
    fresh();
}

void DebugChannelLow::do_key_left_right(int d)
{
    QList<int> list;
    switch (row) {
    case 1:         //L1噪声
        if(sql_c1 != NULL){
            sql_c1->offset_noise += d;
        }
        break;
    case 2:         //L1摄像头/传感器中心频率
        if(sql_para->menu_l1 == AA1){
            sql_c1->camera = !sql_c1->camera;
            if(sql_c1->chart == Camera && sql_c1->camera == false){       //关闭摄像头，把显示界面重置
                sql_c1->chart = BASIC;
            }
        }
        else if(sql_para->menu_l1 == AE1){
            list.clear();
            list << ae_factor_30k << ae_factor_40k << ae_factor_50k << ae_factor_60k
                 << ae_factor_70k << ae_factor_80k << ae_factor_90k << ae_factor_30k_v2 << ae_factor_40k_v2;
            Common::change_index(sql_c1->sensor_freq, d, list);
        }
        break;
    case 3:         //L1包络线
        if(sql_c1 != NULL){
            sql_c1->envelope = !sql_c1->envelope;
        }
        break;
    case 4:         //L1噪声
        if(sql_c2 != NULL){
            sql_c2->offset_noise += d;
        }
        break;
    case 5:         //L2摄像头/传感器中心频率
        if(sql_para->menu_l2 == AA2){
            sql_c2->camera = !sql_c2->camera;
            if(sql_c2->chart == Camera && sql_c2->camera == false){       //关闭摄像头，把显示界面重置
                qDebug()<<"sql_para->menu_l2";
                sql_c2->chart = BASIC;
            }
        }
        else if(sql_para->menu_l2 == AE2){
            list.clear();
            list << ae_factor_30k << ae_factor_40k << ae_factor_50k << ae_factor_60k
                 << ae_factor_70k << ae_factor_80k << ae_factor_90k << ae_factor_30k_v2 << ae_factor_40k_v2;
            Common::change_index(sql_c2->sensor_freq, d, list);
        }
        break;
    case 6:         //L2包络线
        if(sql_c2 != NULL){
            sql_c2->envelope = !sql_c2->envelope;
        }
        break;
    default:
        break;
    }
    fresh();
}

void DebugChannelLow::fresh()
{
    switch (sql_para->menu_l1) {
    case AA1:
        ui->checkBox_camera_L1->setChecked(sql_para->aa1_sql.camera);
        ui->lineEdit_L1_offset->setText(QString("%1").arg(sql_para->aa1_sql.offset_noise ) );
        ui->rbt_L1_envelope->setChecked(sql_para->aa1_sql.envelope);
        ui->rbt_L1_original->setChecked(!sql_para->aa1_sql.envelope);
        break;
    case AE1:
        ui->lineEdit_L1_Freq->setText(Common::sensor_freq_to_string(sql_para->ae1_sql.sensor_freq ) );
        ui->lineEdit_L1_offset->setText(QString("%1").arg(sql_para->ae1_sql.offset_noise ) );
        ui->rbt_L1_envelope->setChecked(sql_para->ae1_sql.envelope);
        ui->rbt_L1_original->setChecked(!sql_para->ae1_sql.envelope);
        break;
    default:
        break;
    }
    switch (sql_para->menu_l2) {
    case AA2:
        ui->checkBox_camera_L2->setChecked(sql_para->aa2_sql.camera);
        ui->lineEdit_L2_offset->setText(QString("%1").arg(sql_para->aa2_sql.offset_noise ) );
        ui->rbt_L2_envelope->setChecked(sql_para->aa2_sql.envelope);
        ui->rbt_L2_original->setChecked(!sql_para->aa2_sql.envelope);
        break;
    case AE2:
        ui->lineEdit_L2_Freq->setText(Common::sensor_freq_to_string(sql_para->ae2_sql.sensor_freq ) );
        ui->lineEdit_L2_offset->setText(QString("%1").arg(sql_para->ae2_sql.offset_noise ) );
        ui->rbt_L2_envelope->setChecked(sql_para->ae2_sql.envelope);
        ui->rbt_L2_original->setChecked(!sql_para->ae2_sql.envelope);
        break;
    default:
        break;
    }

    ui->lineEdit_L1_offset->deselect();
    ui->lineEdit_L2_offset->deselect();
    ui->lineEdit_L1_Freq->deselect();
    ui->lineEdit_L2_Freq->deselect();
    ui->rbt_L1_envelope->setStyleSheet("");
    ui->rbt_L1_original->setStyleSheet("");
    ui->rbt_L2_envelope->setStyleSheet("");
    ui->rbt_L2_original->setStyleSheet("");

    ui->lab_noise_L1->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_camera_L1->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_freq_L1->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_voice_L1->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_noise_L2->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_camera_L2->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_freq_L2->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_voice_L2->setStyleSheet("QLabel{color:#FFFFFF;}");
    switch (row) {
    case 1:
        ui->lineEdit_L1_offset->selectAll();
        ui->lab_noise_L1->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 2:
        ui->lab_camera_L1->setStyleSheet("QLabel{color:#0EC3FD;}");
        ui->lab_freq_L1->setStyleSheet("QLabel{color:#0EC3FD;}");
        ui->lineEdit_L1_Freq->selectAll();
        break;
    case 3:
        ui->lab_voice_L1->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 4:
        ui->lineEdit_L2_offset->selectAll();
        ui->lab_noise_L2->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 5:
        ui->lineEdit_L2_Freq->selectAll();
        ui->lab_camera_L2->setStyleSheet("QLabel{color:#0EC3FD;}");
        ui->lab_freq_L2->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 6:
        ui->lab_voice_L2->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    default:
        break;
    }
}

void DebugChannelLow::timerEvent(QTimerEvent *)
{
    switch (sql_para->menu_l1) {
    case AA1:
        ui->lab_offset_noise_L1->setText(QString("%1").arg(Common::rdb_get_yc_value(AA1_noise_biased_adv_yc)));
        break;
    case AE1:
        ui->lab_offset_noise_L1->setText(QString("%1").arg(Common::rdb_get_yc_value(AE1_noise_biased_adv_yc)));
        break;
    default:
        break;
    }
    switch (sql_para->menu_l2) {
    case AA2:
        ui->lab_offset_noise_L2->setText(QString("%1").arg(Common::rdb_get_yc_value(AA2_noise_biased_adv_yc)));
        break;
    case AE2:
        ui->lab_offset_noise_L2->setText(QString("%1").arg(Common::rdb_get_yc_value(AE2_noise_biased_adv_yc)));
        break;
    default:
        break;
    }
}
