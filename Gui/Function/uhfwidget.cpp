#include "uhfwidget.h"
#include "ui_uhfwidget.h"

UHFWidget::UHFWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::UHFWidget)
{
    ui->setupUi(this);

    this->resize(CHANNEL_X, CHANNEL_Y);
    this->move(3, 3);
    this->setStyleSheet("UHFWidget {border-image: url(:/widgetphoto/bk/bk2.png);}");
    Common::set_comboBox_style(ui->comboBox);

    this->data = data;
    this->key_val = val;
    this->mode = mode;
    this->menu_index = menu_index;

    reload(-1);

    db = 0;
    max_db = 0;
    pulse_cnt_last = 0;
    db_last1 = 0;
    db_last2 = 0;
    manual = false;
}

UHFWidget::~UHFWidget()
{
    delete ui;
}

void UHFWidget::reload(int index)
{

}

void UHFWidget::trans_key(quint8 key_code)
{

}

void UHFWidget::showWaveData(VectorList buf, MODE mod)
{

}

void UHFWidget::fresh_plot()
{

}

void UHFWidget::fresh_PRPD()
{

}

void UHFWidget::fresh_Histogram()
{

}

void UHFWidget::maxReset()
{

}

void UHFWidget::transData(int &x, int &y)
{

}

void UHFWidget::PRPDReset()
{

}

void UHFWidget::calc_tev_value(double *tev_val, double *tev_db, int *sug_central_offset, int *sug_offset)
{

}

void UHFWidget::do_key_up_down(int d)
{

}

void UHFWidget::do_key_left_right(int d)
{

}

void UHFWidget::chart_ini()
{

}

void UHFWidget::fresh_setting()
{

}
