﻿#include "quickview.h"
#include <QGuiApplication>
//#include <QBitmap>
#include "IO/Key/key.h"

#define INTERVAL    10

#define X_INIT      45
//#define Y_INIT      50
#define Y_INIT      100

#define X_SPACE      39
#define Y_SPACE      33

#define H_MODIFY    50


QuickView::QuickView(QString str): QQuickView(str)
{
    this->setResizeMode(QQuickView::SizeRootObjectToView);
    this->setGeometry(0,H_MODIFY,480,272-H_MODIFY);

    //主计时器
    timer1 = new QTimer;
    timer1->setSingleShot(true);
    timer1->setInterval(1000);
    timer1->start();
//    connect(timer1,SIGNAL(timeout()), this, SLOT(test_press()) );
    connect(timer1,SIGNAL(timeout()), this, SLOT(view_init()) );

    timer_short = new QTimer;
    timer_short->setInterval(200);
    timer_short->setSingleShot(true);
    connect(timer_short,SIGNAL(timeout()), this, SLOT(press2()) );


//    n = 450*30/INTERVAL/INTERVAL;

    _x = X_INIT;
    _y = Y_INIT;


//    space = 40;

    isWorking = false;

}

//void QuickView::test_press()
//{
//    n++;
//    if(n > 450 * 260){
//        timer1->stop();

//    }
//    else{
//        _x = (n % (450/INTERVAL) ) * INTERVAL;
//        _y = (n / (260/INTERVAL) ) * INTERVAL;
////        _y = 130;

//        QPoint pos(_x, _y);

//        QMouseEvent event0(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//        QGuiApplication::sendEvent(this, &event0);

//        timer_short->start();
//    }
//}

void QuickView::press1()
{
    QPoint pos(_x, _y);

    QMouseEvent event0(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QGuiApplication::sendEvent(this, &event0);

    timer_short->start();
}

void QuickView::press2()
{
    QPoint pos(_x, _y);
    QMouseEvent event1(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QGuiApplication::sendEvent(this, &event1);
    qDebug()<<"mouse clicked:"<< pos.x() <<'\t'<< pos.y();

    emit mouseClicked(_x,_y);

    //    test_press();
}

void QuickView::view_init()
{
    emit viewInit(H_MODIFY);
}

//初始化
void QuickView::show_input(QString str1,QString str2)
{
    this->show();
    isWorking = true;
    _x = X_INIT;
    _y = Y_INIT;
    emit inputClear(str1, str2);
    refresh();
}

void QuickView::editFinished(QString str)
{
    if(isWorking){
//        qDebug()<<"QuickView recv str:"<<str;
        emit input_str(str);
        this->hide();
        isWorking = false;
    }
}

void QuickView::qml_view_init(int y_init)
{
    _y = y_init + 6;
    y_min = y_init;
//    qDebug()<<"y_init:"<<y_init;
}

void QuickView::trans_input_key(quint8 key_code)
{
    switch (key_code) {
    case KEY_OK:
        press1();
        break;
    case KEY_CANCEL:
        editFinished("");
        break;
    case KEY_UP:
        do_key_up_down(-1);
        break;
    case KEY_DOWN:
        do_key_up_down(1);
        break;
    case KEY_LEFT:
        do_key_left_right(-1);
        break;
    case KEY_RIGHT:
        do_key_left_right(1);
        break;
    default:
        break;
    }
    refresh();
}

void QuickView::do_key_up_down(int d)
{
    _y += d * Y_SPACE;
    _x += d * 16;

    if(_x > 460){
        _x -= 11 * X_SPACE;
    }

    if(_x < 30){
        _x += 11 * X_SPACE;
    }

//    if(_y > (y_min+180)){
//        _y -= 4 * Y_SPACE;
//        _x -= 4 * 16;
//    }

//    if(_y < y_min){
//        _y += 4 * Y_SPACE;
//        _x += 4 * 16;
//    }
}

void QuickView::do_key_left_right(int d)
{
    _x += d * X_SPACE;

    if(_x > 440){
        _x -= 11 * X_SPACE;
    }

    if(_x < 30){
        _x += 11 * X_SPACE;
    }
}

void QuickView::refresh()
{
    emit mouseMoved(_x,_y);
//    qDebug()<<"mouse moved:"<< _x <<'\t'<< _y;
}















