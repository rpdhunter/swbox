#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include "Gui/Common/basewidget.h"

namespace Ui {
class SystemInfoUi;
}

class SystemInfo : public BaseWidget
{
    Q_OBJECT
public:
    SystemInfo(QWidget *parent = 0);

    void working(CURRENT_KEY_VALUE *val);

public slots:
    void trans_key(quint8);

private:
    Ui::SystemInfoUi *ui;

    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int){}
    void do_key_left_right(int){}
};

#endif // SYSTEMINFO_H
