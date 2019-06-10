#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QWidget>
#include "IO/Data/data.h"
#include "IO/SqlCfg/sqlcfg.h"

class TabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidget(SQL_PARA *sql, QWidget *parent = nullptr);

//    virtual void work(int d);
    virtual void do_key_ok() = 0;
    virtual void do_key_cancel() = 0;
    virtual void do_key_up_down(int d) = 0;
    virtual void do_key_left_right(int d) = 0;
    virtual void fresh() = 0;

signals:
    void quit();        //完全退出的信号
    void save();        //保存设置
    void updata_tooltip(QString tooltip,int time);  //更新通知

public slots:

protected:
    SQL_PARA *sql_para;
    int row, col;
    QString pbt_style_unselect, pbt_style_select;
};

#endif // TABWIDGET_H
