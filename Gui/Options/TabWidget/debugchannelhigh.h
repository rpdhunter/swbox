#ifndef DEBUGCHANNELHIGH_H
#define DEBUGCHANNELHIGH_H

#include "tabwidget.h"

namespace Ui {
class DebugChannelHigh;
}

class DebugChannelHigh : public TabWidget
{
    Q_OBJECT

public:
    explicit DebugChannelHigh(SQL_PARA *sql, QWidget *parent = 0);
    ~DebugChannelHigh();

    void work();
    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void fresh();

protected:
    void timerEvent(QTimerEvent *);

private:
    Ui::DebugChannelHigh *ui;

    CHANNEL_SQL *sql_c1, *sql_c2;       //通道sql
    int chk_status_h1, chk_status_h2;       //表示复选框fir和wavelet的选择状态,00代表两复选框都不选择,10代表fir选择,01代表wavelet选择

};

#endif // DEBUGCHANNELHIGH_H
