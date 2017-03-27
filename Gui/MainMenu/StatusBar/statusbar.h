#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QFrame>
#include <QLabel>
#include <QDebug>
#include <QDate>
#include <QTime>
#include <QTimer>

//完成主界面上的状态栏绘制
class StatusBar : public QFrame
{
    Q_OBJECT
public:
    StatusBar(QWidget *parent = 0);

private:
    QLabel *time, *batt;
    QTimer *timer;

    qint8 batt_val;

private slots:
    void fresh_time(void);
    void fresh_batt(void);
};

#endif // STATUSBAR_H
