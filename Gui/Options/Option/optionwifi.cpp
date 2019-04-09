#include "optionwifi.h"
#include "ui_optionwifi.h"

OptionWifi::OptionWifi(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionWifi)
{
    ui->setupUi(this);

    row = 0;
    col = 0;
    fresh();
}

OptionWifi::~OptionWifi()
{
    delete ui;
}

void OptionWifi::work(int d)
{
    col = 1;
    row = 1;

    fresh();
}

void OptionWifi::do_key_ok()
{
    row = 0;
    col = 0;
    fresh();
    emit save();
    emit quit();
}

void OptionWifi::do_key_cancel()
{
    fresh();
    emit quit();
}

void OptionWifi::do_key_up_down(int d)
{
    fresh();
}

void OptionWifi::do_key_left_right(int d)
{
    fresh();
}

void OptionWifi::fresh()
{

}
