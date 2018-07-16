#ifndef AMPLITUDE1_H
#define AMPLITUDE1_H

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

public slots:
    void reload(int index);   //重新装载设置
    void save_channel();        //保存通道数据

private slots:
    void fresh_1000ms();
    void fresh_100ms();
    void fresh_1ms();

private:
    void do_key_ok();
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void fresh_setting();

    void chart_ini();
    void data_reset();
    void fresh_Histogram();
    void calc_tev_value (double &tev_db, int &pulse_cnt_show, double &degree, int &sug_central_offset, int &sug_noise_offset);

    Ui::TEVWidget *ui;
    H_CHANNEL_SQL *tev_sql;
};

#endif // AMPLITUDE1_H
