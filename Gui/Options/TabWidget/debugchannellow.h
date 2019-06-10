#ifndef DEBUGCHANNELLOW_H
#define DEBUGCHANNELLOW_H

#include "tabwidget.h"

namespace Ui {
class DebugChannelLow;
}

class DebugChannelLow : public TabWidget
{
    Q_OBJECT

public:
    explicit DebugChannelLow(SQL_PARA *sql, QWidget *parent = 0);
    ~DebugChannelLow();

    void work();
    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void fresh();

protected:
    void timerEvent(QTimerEvent *);

private:
    Ui::DebugChannelLow *ui;

    CHANNEL_SQL *sql_c1, *sql_c2;       //通道sql


};

#endif // DEBUGCHANNELLOW_H
