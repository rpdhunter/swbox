#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QFrame>

class Battery;
class QLabel;
class QTimer;

//完成主界面上的状态栏绘制
class StatusBar : public QFrame
{
    Q_OBJECT
public:
    StatusBar(QWidget *parent = 0);

private slots:
    void fresh_time(void);
    void fresh_batt(void);

private:
    QLabel *time, *batt;
    QTimer *timer_time,*timer_batt;

    int batt_val;

    Battery *battery;
};

#endif // STATUSBAR_H
