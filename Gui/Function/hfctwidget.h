#ifndef HFCTWIDGET_H
#define HFCTWIDGET_H

#include "channelwidget.h"
#include "Algorithm/Bp/bpcable.h"

namespace Ui {
class HFCTWidget;
}

class HFCTWidget : public ChannelWidget
{
    Q_OBJECT
public:
    explicit HFCTWidget(G_PARA *data, CURRENT_KEY_VALUE *val, MODE mode, int menu_index, QWidget *parent = 0);
    ~HFCTWidget();

private slots:
    void fresh_100ms();
    void fresh_1000ms();    

private:
    Ui::HFCTWidget *ui;
    BpCable *bpcable;

    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void data_reset();
//    void fresh_setting(void);
};

#endif // HFCTWIDGET_H
