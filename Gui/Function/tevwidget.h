#ifndef TEVWIDGET_H
#define TEVWIDGET_H

#include "channelwidget.h"

namespace Ui {
class TEVWidget;
}

class TEVWidget : public ChannelWidget
{
    Q_OBJECT
public:
    explicit TEVWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = 0);
    ~TEVWidget();

private slots:
    void fresh_100ms();
    void fresh_1000ms();

private:
    Ui::TEVWidget *ui;

    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void data_reset();
};

#endif // TEVWIDGET_H
