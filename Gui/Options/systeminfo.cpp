#include "systeminfo.h"
#include "ui_systeminfo.h"

SystemInfo::SystemInfo(QWidget *parent) : QFrame(parent),ui(new Ui::SystemInfoUi)
{
    ui->setupUi(this);
    this->setStyleSheet("SystemInfo {background-color:lightGray;}");
    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);

    key_val = NULL;

#ifdef OHV

#elif AMG
    ui->label_logo_text->setText("copyright © 2016-2017 Australian Microgrid Technology Pty Lte\nAll rights reserved.");
    ui->label_logo->setPixmap(QPixmap(":/widgetphoto/bk/amg_low.png"));
#else
    ui->label_logo_text->setText("");
    ui->label_logo->hide();
#endif

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

