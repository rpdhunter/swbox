#include "systeminfo.h"
#include "ui_systeminfo.h"

SystemInfo::SystemInfo(QWidget *parent) : QFrame(parent),ui(new Ui::SystemInfoUi)
{
    key_val = NULL;

    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);

    ui->setupUi(this);
    this->setStyleSheet("SystemInfo {background-color:lightGray;}");

    this->hide();
}

void SystemInfo::working(CURRENT_KEY_VALUE *val)
{
    if (val == NULL) {
        return;
    }
    key_val = val;

    this->show();
}

void SystemInfo::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if (key_val->grade.val1 != 4) {
        return;
    }

    switch (key_code) {
    case KEY_OK:
        key_val->grade.val2 = 0;
        fresh_parent();
        this->hide();
        break;
    case KEY_CANCEL:
        key_val->grade.val2 = 0;
        fresh_parent();
        this->hide();
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
}

