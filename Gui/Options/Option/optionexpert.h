#ifndef OPTIONEXPERT_H
#define OPTIONEXPERT_H

#include <QWidget>
#include "IO/SqlCfg/sqlcfg.h"
#include "Thread/Sync/syncsocket.h"

namespace Ui {
class OptionExpert;
}

class OptionExpert : public QWidget
{
    Q_OBJECT

public:
    explicit OptionExpert(SQL_PARA *sql,QWidget *parent = 0);
    ~OptionExpert();

    void work();
    void do_key_ok();
    void do_key_cancel();
    void do_key_up_down(int d);
    void do_key_left_right(int d);    

public slots:
    void update_status(QString str);
    void fresh();               //刷新UI

signals:
    void quit();        //完全退出的信号
    void save();        //保存设置

private:
    Ui::OptionExpert *ui;
    int row;
    SQL_PARA *sql_para;
    SyncSocket *sync_socket;

    void set_sync_status();     //设置sync线程状态


};

#endif // OPTIONEXPERT_H
