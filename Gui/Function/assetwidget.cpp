#include "assetwidget.h"
#include "ui_assetwidget.h"
#include <QLineEdit>

#define SETTING_NUM 7           //设置菜单条目数

AssetWidget::AssetWidget(CURRENT_KEY_VALUE *val, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::AssetWidget)
{
    ui->setupUi(this);
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("AssetWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    Common::set_comboBox_style(ui->comboBox);

    this->key_val = val;
    this->menu_index = menu_index;
}

AssetWidget::~AssetWidget()
{
    delete ui;
}

void AssetWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL || key_val->grade.val0 != menu_index) {
        return;
    }
//    qDebug()<<"val0 = "<<key_val->grade.val0 <<"\tval1 = "<<key_val->grade.val1 <<"\tval2 = "<<key_val->grade.val2 ;
    switch (key_code) {
    case KEY_OK:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_CANCEL:
        key_val->grade.val1 = 0;
        key_val->grade.val2 = 0;
        break;
    case KEY_UP:
        do_key_up_down(-1);
        break;
    case KEY_DOWN:
        do_key_up_down(1);
        break;
    case KEY_LEFT:
        do_key_left_right(-1);
        break;
    case KEY_RIGHT:
        do_key_left_right(1);
        break;
    default:
        break;
    }
    emit fresh_parent();
    fresh_setting();
}

void AssetWidget::do_key_up_down(int d)
{
    key_val->grade.val1 = 1;
    Common::change_index(key_val->grade.val2, d, SETTING_NUM, 1);
}

void AssetWidget::do_key_left_right(int d)
{
    switch (key_val->grade.val2) {
    case 1:
        break;
    case 2:
        break;
    default:
        break;
    }
}

void AssetWidget::fresh_setting()
{
    ui->comboBox->setCurrentIndex(key_val->grade.val2-1);

    if (key_val->grade.val2 && key_val->grade.val0 == menu_index && key_val->grade.val5 == 0) {
        ui->comboBox->showPopup();
    }
    else{
        ui->comboBox->hidePopup();
    }

    ui->comboBox->lineEdit()->setText(tr(" 参 数 设 置"));
}
