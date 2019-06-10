#include "debugchannelhigh.h"
#include "ui_debugchannelhigh.h"
#include "Gui/Common/common.h"

DebugChannelHigh::DebugChannelHigh(SQL_PARA *sql, QWidget *parent) :
    TabWidget(sql,parent),
    ui(new Ui::DebugChannelHigh)
{
    ui->setupUi(this);
    this->resize(TABWIDGET_X,TABWIDGET_Y);

    switch (sql_para->menu_h1) {
    case TEV1:
        sql_c1 = &sql_para->tev1_sql;
        break;
    case HFCT1:
        sql_c1 = &sql_para->hfct1_sql;
        break;
    case UHF1:
        sql_c1 = &sql_para->uhf1_sql;
        break;
    default:
        sql_c1 = NULL;
    }

    switch (sql_para->menu_h2) {
    case TEV2:
        sql_c2 = &sql_para->tev2_sql;
        break;
    case HFCT2:
        sql_c2 = &sql_para->hfct2_sql;
        break;
    case UHF2:
        sql_c2 = &sql_para->uhf2_sql;
        break;
    default:
        sql_c2 = NULL;
    }

    QString style = "QLabel {font-family:WenQuanYi Micro Hei;font: bold; font-size:16px;color:green}";
    if(sql_c1 != NULL){
        ui->label_H1->setStyleSheet(style);
        ui->label_H1->setText(Common::mode_to_string((MODE)sql_para->menu_h1));
    }
    if(sql_c2 != NULL){
        ui->label_H2->setStyleSheet(style);
        ui->label_H2->setText(Common::mode_to_string((MODE)sql_para->menu_h2));

    }

    chk_status_h1 = 00;
    chk_status_h2 = 00;

    startTimer(1000);

}

DebugChannelHigh::~DebugChannelHigh()
{
    delete ui;
}

void DebugChannelHigh::work()
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

void DebugChannelHigh::do_key_ok()
{
    //切换复选框状态
    if(row == 3){
        if(chk_status_h1 == 10 && sql_c1 != NULL){
            sql_c1->filter_fir_fpga = !sql_c1->filter_fir_fpga;
        }
        else if(chk_status_h1 == 01 && sql_c1 != NULL){
            sql_c1->filter_wavelet = !sql_c1->filter_wavelet;
        }
    }
    if(row == 6){
        if(chk_status_h2 == 10 && sql_c2 != NULL){
            sql_c2->filter_fir_fpga = !sql_c2->filter_fir_fpga;
        }
        else if(chk_status_h2 == 01 && sql_c2 != NULL){
            sql_c2->filter_wavelet = !sql_c2->filter_wavelet;
        }
    }

    emit save();
    fresh();
}

void DebugChannelHigh::do_key_cancel()
{
    row = 0;
    chk_status_h1 = 00;
    chk_status_h2 = 00;
    fresh();
    emit quit();
}

void DebugChannelHigh::do_key_up_down(int d)
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

    switch (row) {
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
    fresh();
}

void DebugChannelHigh::do_key_left_right(int d)
{
    switch (row) {
    case 1:         //H1零偏置
        if(sql_c1 != NULL){
            sql_c1->fpga_zero += d;
        }
        break;
    case 2:         //H1噪声偏置
        if(sql_c1 != NULL){
            sql_c1->offset_noise += d;
        }
        break;
    case 3:         //H1前置滤波
        Common::change_value(chk_status_h1, 01, 10);
        break;
    case 4:         //H2零偏置
        if(sql_c2 != NULL){
            sql_c2->fpga_zero += d;
        }
        break;
    case 5:         //H2噪声偏置
        if(sql_c2 != NULL){
            sql_c2->offset_noise += d;
        }
        break;
    case 6:         //H2前置滤波
        Common::change_value(chk_status_h2, 01, 10);
        break;
    default:
        break;
    }
    fresh();
}

void DebugChannelHigh::fresh()
{
    if(sql_c1 != NULL){
        ui->lineEdit_H1_ZERO->setText(QString("%1").arg(sql_c1->fpga_zero ) );
        ui->lineEdit_H1_NOISE->setText(QString("%1").arg(sql_c1->offset_noise ) );
        ui->cbx_FIR_h1->setChecked(sql_c1->filter_fir_fpga);
        ui->cbx_wavelet_h1->setChecked(sql_c1->filter_wavelet);
    }

    if(sql_c2 != NULL){
        ui->lineEdit_H2_ZERO->setText(QString("%1").arg(sql_c2->fpga_zero ) );
        ui->lineEdit_H2_NOISE->setText(QString("%1").arg(sql_c2->offset_noise ) );
        ui->cbx_FIR_h2->setChecked(sql_c2->filter_fir_fpga);
        ui->cbx_wavelet_h2->setChecked(sql_c2->filter_wavelet);
    }

    if(chk_status_h1 == 00){
        ui->cbx_FIR_h1->setTextColor("#FFFFFF");
        ui->cbx_wavelet_h1->setTextColor("#FFFFFF");
    }
    else if(chk_status_h1 == 10){
        ui->cbx_FIR_h1->setTextColor("#0EC3FD");
        ui->cbx_wavelet_h1->setTextColor("#FFFFFF");
    }
    else if(chk_status_h1 == 01){
        ui->cbx_FIR_h1->setTextColor("#FFFFFF");
        ui->cbx_wavelet_h1->setTextColor("#0EC3FD");
    }
    if(chk_status_h2 == 00){
        ui->cbx_FIR_h2->setTextColor("#FFFFFF");
        ui->cbx_wavelet_h2->setTextColor("#FFFFFF");
    }
    else if(chk_status_h2 == 10){
        ui->cbx_FIR_h2->setTextColor("#0EC3FD");
        ui->cbx_wavelet_h2->setTextColor("#FFFFFF");
    }
    else if(chk_status_h2 == 01){
        ui->cbx_FIR_h2->setTextColor("#FFFFFF");
        ui->cbx_wavelet_h2->setTextColor("#0EC3FD");
    }

    ui->lineEdit_H1_ZERO->deselect();
    ui->lineEdit_H1_NOISE->deselect();
    ui->lineEdit_H2_ZERO->deselect();
    ui->lineEdit_H2_NOISE->deselect();
    ui->lab_zero_h1->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_noise_h1->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_filter_h1->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_zero_h2->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_noise_h2->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_filter_h2->setStyleSheet("QLabel{color:#FFFFFF;}");
    switch (row) {
    case 1:
        ui->lineEdit_H1_ZERO->selectAll();
        ui->lab_zero_h1->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 2:
        ui->lineEdit_H1_NOISE->selectAll();
        ui->lab_noise_h1->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 3:
        ui->lab_filter_h1->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 4:
        ui->lineEdit_H2_ZERO->selectAll();
        ui->lab_zero_h2->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 5:
        ui->lineEdit_H2_NOISE->selectAll();
        ui->lab_noise_h2->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 6:
        ui->lab_filter_h2->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    default:
        break;
    }
}

void DebugChannelHigh::timerEvent(QTimerEvent *)
{
    switch (sql_para->menu_h1) {
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

    switch (sql_para->menu_h2) {
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
}
