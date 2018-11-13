#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QFrame>
#include "IO/Key/key.h"
#include <QTimer>

class BaseWidget : public QFrame
{
    Q_OBJECT
public:
    explicit BaseWidget(CURRENT_KEY_VALUE *val = NULL, QWidget *parent = nullptr);

signals:
    void fresh_parent();
    void send_key(quint8);
    void show_indicator(bool);      //显示菊花
    void update_statusBar(QString);

protected slots:
    virtual void trans_key(quint8 key_code);
    void do_busy(bool f);   //改变isBusy状态
    void reset_indicator();     //超时重置菊花

protected:
    virtual void do_key_up_down(int d) = 0;
    virtual void do_key_left_right(int d) = 0;
    virtual void do_key_ok() = 0;
    virtual void do_key_cancel() = 0;


    CURRENT_KEY_VALUE *key_val;
    bool isBusy;            //菊花状态
    QTimer *timer_busy;

};

#endif // BASEWIDGET_H
