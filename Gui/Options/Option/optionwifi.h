#ifndef OPTIONWIFI_H
#define OPTIONWIFI_H

#include <QWidget>

namespace Ui {
class OptionWifi;
}

class OptionWifi : public QWidget
{
    Q_OBJECT

public:
    explicit OptionWifi(QWidget *parent = 0);
    ~OptionWifi();

    void work(int d);
    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);

signals:
    void quit();        //完全退出的信号
    void save();        //保存设置

private:
    Ui::OptionWifi *ui;
    int row,col;
    void fresh();

};

#endif // OPTIONWIFI_H
