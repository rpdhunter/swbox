#include "statusbar.h"
#include "battery.h"
#include <QLabel>
//#include <QDebug>
#include <QDate>
#include <QTime>
#include <QTimer>

StatusBar::StatusBar(QWidget *parent) : QFrame(parent)
{
    batt_val = 10;                                                              //batt is full

    battery = new Battery;

    this->resize(480, 24);
    this->move(10, 0);

    /* view date */
    time = new QLabel(this);
    time->resize(200, 20);
    time->move(5, 5);
    time->setAlignment(Qt::AlignVCenter);
    time->setStyleSheet("QLabel {color:white;}");

    batt = new QLabel(this);
    batt->resize(25, 9);
    batt->move(this->width() - 27 - batt->width(), 10);

    timer_time = new QTimer();
    timer_time->setInterval(1000);   //1秒1跳
    timer_time->start();

    timer_batt = new QTimer();
    timer_batt->setInterval(10000);   //10秒1跳
    timer_batt->start();

    /* refresh time */
    connect(timer_time, &QTimer::timeout, this, &StatusBar::fresh_time);

    /* refresh batt */
    connect(timer_batt, &QTimer::timeout, this, &StatusBar::fresh_batt);

    fresh_batt();       //立刻刷新一次


}

//刷新时间
void StatusBar::fresh_time(void)
{
    time->setText(QDate::currentDate().toString("yyyy年M月d日")
                  + " "
                  + QTime::currentTime().toString("h:mm:ss"));
}

//刷新电池电量
//永远为9，原来是显示的假电源
void StatusBar::fresh_batt(void)
{
    batt_val = battery->battValue() / 10;
//    qDebug()<<"curr battery value:"<<batt_val;
    switch (batt_val) {
    case 0:
        batt->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr0.png);}");
        break;
    case 1:
        batt->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr10.png);}");
        break;
    case 2:
        batt->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr20.png);}");
        break;
    case 3:
        batt->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr30.png);}");
        break;
    case 4:
        batt->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr40.png);}");
        break;
    case 5:
        batt->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr50.png);}");
        break;
    case 6:
        batt->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr60.png);}");
        break;
    case 7:
        batt->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr70.png);}");
        break;
    case 8:
        batt->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr80.png);}");
        break;
    case 9:
        batt->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr90.png);}");
        break;
    case 10:
        batt->setStyleSheet("QLabel {border-image: url(:/widgetphoto/pwr/pwr100.png);}");
        break;
    default:
        break;
    }
}
