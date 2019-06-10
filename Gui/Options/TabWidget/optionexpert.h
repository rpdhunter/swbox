#ifndef OPTIONEXPERT_H
#define OPTIONEXPERT_H

#include "tabwidget.h"
#include "Thread/Sync/syncsocket.h"
#include "Thread/Sync/syncclamp.h"

namespace Ui {
class OptionExpert;
}

class OptionExpert : public TabWidget
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
    void deal_gps_info(GPSInfo *gps);

signals:
//    void quit();        //完全退出的信号
//    void save();        //保存设置

    void fregChanged(float );
    void gpsChanged(QString );
    void send_sync(qint64,qint64);          //发送同步时间（秒，微秒）
    void set_sync_source(int);          //设置同步源

private:
    Ui::OptionExpert *ui;
    SyncSocket *sync_socket;
    SyncClamp *_sync_clamp;
    QThread *thread;

    void set_sync_status();     //设置sync线程状态


};

#endif // OPTIONEXPERT_H
