#ifndef UHFWIDGET_H
#define UHFWIDGET_H

#include "channelwidget.h"

namespace Ui {
class UHFWidget;
}

class UHFWidget : public ChannelWidget
{
    Q_OBJECT
public:
    explicit UHFWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = 0);
    ~UHFWidget();

private slots:
    void fresh_100ms();
    void fresh_1000ms();

private:
    Ui::UHFWidget *ui;

    void do_key_ok();
//    void do_key_cancel();
//    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void data_reset();
};

#endif // UHFWIDGET_H
