#include "debughardware.h"
#include "ui_debughardware.h"
#include "Gui/Common/common.h"

DebugHardware::DebugHardware(SQL_PARA *sql, QWidget *parent) :
    TabWidget(sql,parent),
    ui(new Ui::DebugHardware)
{
    ui->setupUi(this);
    this->resize(TABWIDGET_X,TABWIDGET_Y);

    ui->lineEdit_CompileTime->setText(QString("%1 %2").arg(__TIME__).arg(__DATE__));

    startTimer(1000);
}

DebugHardware::~DebugHardware()
{
    delete ui;
}

void DebugHardware::work()
{
    row = 1;
    fresh();
}

void DebugHardware::do_key_ok()
{
    fresh();
    emit save();
}

void DebugHardware::do_key_cancel()
{
    row = 0;
    fresh();
    emit quit();
}

void DebugHardware::do_key_up_down(int)
{

}

void DebugHardware::do_key_left_right(int)
{

}

void DebugHardware::fresh()
{
    ui->lineEdit_CPU_TEMP->setText( QString::number(Common::rdb_get_yc_value(CPU_temp_yc), 'f', 2) );
    ui->lineEdit_CPU_VCC->setText( QString::number(Common::rdb_get_yc_value(CPU_vcc_yc), 'f', 2) );
    float v = Common::rdb_get_yc_value(Battery_vcc_yc), c = Common::rdb_get_yc_value(Battery_cur_yc);
    ui->lineEdit_BATT_VCC->setText( QString::number(v, 'f', 2) );
    ui->lineEdit_BATT_CUR->setText( QString::number(c, 'f', 2) );
    ui->lineEdit_BATT_P->setText( QString::number(v*c/1000, 'f', 2) );
}

void DebugHardware::timerEvent(QTimerEvent *)
{
    fresh();
}
