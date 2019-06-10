#ifndef DEBUGHARDWARE_H
#define DEBUGHARDWARE_H

#include "tabwidget.h"

namespace Ui {
class DebugHardware;
}

class DebugHardware : public TabWidget
{
    Q_OBJECT

public:
    explicit DebugHardware(SQL_PARA *sql, QWidget *parent = 0);
    ~DebugHardware();

    void work();
    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int);
    void do_key_left_right(int);
    void fresh();

protected:
    void timerEvent(QTimerEvent *);

private:
    Ui::DebugHardware *ui;
};

#endif // DEBUGHARDWARE_H
