#include "debugchannelsetting.h"
#include "ui_debugchannelsetting.h"
#include "Gui/Common/common.h"

DebugChannelSetting::DebugChannelSetting(SQL_PARA *sql, QWidget *parent) :
    TabWidget(sql,parent),
    ui(new Ui::DebugChannelSetting)
{
    ui->setupUi(this);
    this->resize(TABWIDGET_X,TABWIDGET_Y);

    QButtonGroup *group1 = new QButtonGroup(this);
    QButtonGroup *group2 = new QButtonGroup(this);
    QButtonGroup *group3 = new QButtonGroup(this);
    QButtonGroup *group4 = new QButtonGroup(this);
    QButtonGroup *group5 = new QButtonGroup(this);
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


}

DebugChannelSetting::~DebugChannelSetting()
{
    delete ui;
}

void DebugChannelSetting::work()
{
    row = 1;
    fresh();
}

void DebugChannelSetting::do_key_ok()
{
    fresh();
    emit save();
}

void DebugChannelSetting::do_key_cancel()
{
    row = 0;
    fresh();
    emit quit();
}

void DebugChannelSetting::do_key_up_down(int d)
{
    Common::change_index(row, d, 6, 1);
    fresh();
}

void DebugChannelSetting::do_key_left_right(int d)
{    
    QList<int> list;
    switch (row) {
    case 1:     //高频通道1
        list << Disable << TEV1 << HFCT1 << UHF1;
        Common::change_index(sql_para->menu_h1, d, list);
        break;
    case 2:     //高频通道2
        list << Disable << TEV2 << HFCT2 << UHF2;
        Common::change_index(sql_para->menu_h2, d, list);
        break;
    case 3:     //低频通道1
        list << Disable << AA1 << AE1;
        Common::change_index(sql_para->menu_l1, d, list);
        break;
    case 4:     //低频通道1
        list << Disable << AA2 << AE2;
        Common::change_index(sql_para->menu_l2, d, list);
        break;
    case 5:     //双通道
        list << Disable << Double_Channel;
        Common::change_index(sql_para->menu_double, d, list);
        break;
    case 6:     //资产
        list << Disable << ASSET;
        Common::change_index(sql_para->menu_asset, d, list);
        break;
    default:
        break;
    }
    fresh();
}

void DebugChannelSetting::fresh()
{
    switch (sql_para->menu_h1) {
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

    switch (sql_para->menu_h2) {
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

    switch (sql_para->menu_l1) {
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

    switch (sql_para->menu_l2) {
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

    switch (sql_para->menu_double) {
    case Double_Channel:
        ui->rb_Double_Enable->setChecked(true);
        break;
    case Disable:
        ui->rb_Double_Disable->setChecked(true);
        break;
    default:
        break;
    }

    switch (sql_para->menu_asset) {
    case ASSET:
        ui->rb_Asset_Enable->setChecked(true);
        break;
    case Disable:
        ui->rb_Asset_Disable->setChecked(true);
        break;
    default:
        break;
    }

    ui->lab_HC1->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_HC2->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_LC1->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_LC2->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_Double->setStyleSheet("QLabel{color:#FFFFFF;}");
    ui->lab_Asset->setStyleSheet("QLabel{color:#FFFFFF;}");
    switch (row) {
    case 1:
        ui->lab_HC1->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 2:
        ui->lab_HC2->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 3:
        ui->lab_LC1->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 4:
        ui->lab_LC2->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 5:
        ui->lab_Double->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    case 6:
        ui->lab_Asset->setStyleSheet("QLabel{color:#0EC3FD;}");
        break;
    default:
        break;
    }
}
