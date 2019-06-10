#ifndef OPTIONBASIC_H
#define OPTIONBASIC_H

#include "tabwidget.h"
#include <QDateTime>

namespace Ui {
class OptionBasic;
}

class OptionBasic : public TabWidget
{
    Q_OBJECT
public:
    explicit OptionBasic(G_PARA *g_data, SQL_PARA *sql, QWidget *parent = 0);
    ~OptionBasic();

    void work();
    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);
    void fresh();

signals:
//    void quit();        //完全退出的信号
//    void save();        //保存设置

private:
    Ui::OptionBasic *ui;
    QDateTime _datetime;
    G_PARA *data;

};

#endif // OPTIONBASIC_H
