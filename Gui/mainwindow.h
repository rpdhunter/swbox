#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFrame>
#include <QLabel>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include "IO/Key/keydetect.h"
#include "MainMenu/mainmenu.h"
//#include "IO/Data/data.h"
#include "IO/Data/fifodata.h"
#include "IO/Modbus/modbus.h"
#include <QPixmap>

class MainWindow : public QFrame
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = NULL);

private:
    KeyDetect *keydetect;
    MainMenu *mainmenu;
    FifoData *fifodata;
    Modbus *modbus;

    G_PARA *g_data;
    QTimer *rebootTimer;    //系统关机时间

    QTimer *showTimer;

//    QPixmap *fullScreenPixmap;



signals:
    void sendkey(quint8);

public slots:
    void system_reboot();
    void setCloseTime(int m);
    void showTime();

    void printSc(); //截屏

protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // MAINWINDOW_H
