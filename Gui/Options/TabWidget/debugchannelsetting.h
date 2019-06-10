#ifndef DEBUGCHANNELSETTING_H
#define DEBUGCHANNELSETTING_H

#include "tabwidget.h"

namespace Ui {
class DebugChannelSetting;
}

class DebugChannelSetting : public TabWidget
{
    Q_OBJECT

public:
    explicit DebugChannelSetting(SQL_PARA *sql, QWidget *parent = 0);
    ~DebugChannelSetting();

    void work();
    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void fresh();

private:
    Ui::DebugChannelSetting *ui;
};

#endif // DEBUGCHANNELSETTING_H
