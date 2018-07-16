#include "basewidget.h"

BaseWidget::BaseWidget(CURRENT_KEY_VALUE *val, QWidget *parent) : QFrame(parent)
{
    this->key_val = val;
    this->isBusy = false;
    connect(this,SIGNAL(show_indicator(bool)), this, SLOT(do_busy(bool)));

    timer_busy = new QTimer;
    timer_busy->setSingleShot(true);
    timer_busy->setInterval(10000);     //无响应复位时间10s
    connect(timer_busy,SIGNAL(timeout()),this,SLOT(reset_indicator()));
}

void BaseWidget::trans_key(quint8 key_code)
{
    if (key_val == NULL) {
        return;
    }

    if(isBusy){
        return;
    }

    switch (key_code) {
    case KEY_OK:
        do_key_ok();
        break;
    case KEY_CANCEL:
        do_key_cancel();
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
        return;
    }
}

void BaseWidget::do_busy(bool f)
{
    isBusy = f;
    if(f){
        timer_busy->start();
    }
    else{
        timer_busy->stop();
    }
}

void BaseWidget::reset_indicator()
{
    emit show_indicator(false);
}
