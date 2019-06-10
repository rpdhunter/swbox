#ifndef AEWIDGET_H
#define AEWIDGET_H

#include "channelwidget.h"

namespace Ui {
class AAWidget;
}

class AEWidget : public ChannelWidget
{
    Q_OBJECT
public:
    explicit AEWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = 0);
    ~AEWidget();

private slots:
    void fresh_1000ms();
    void fresh_gusty();
    void fresh_100ms();

private:
    void do_key_ok();
//    void do_key_cancel();
//    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void data_reset();

    Ui::AAWidget *ui;
};

#endif // AEWIDGET_H








