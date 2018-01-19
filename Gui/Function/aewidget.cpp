#include "aewidget.h"
#include "ui_aewidget.h"

AEWidget::AEWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::AEWidget)
{
    ui->setupUi(this);

    this->resize(CHANNEL_X, CHANNEL_Y);
    this->setStyleSheet("AEWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    this->move(3, 3);

    this->data = data;
    this->mode = mode;
    key_val = val;
    this->menu_index = menu_index;
    sql_para = *sqlcfg->get_para();
}

AEWidget::~AEWidget()
{
    delete ui;
}

void AEWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }
    if (key_val->grade.val0 != menu_index) {
        return;
    }

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
        break;
    case KEY_DOWN:
        break;
    case KEY_LEFT:
        break;
    case KEY_RIGHT:
        break;
    default:
        break;
    }
    emit fresh_parent();
    fresh_setting();
}

void AEWidget::fresh_setting()
{

}
