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
#include "IO/Data/data.h"
#include "IO/Data/fifodata.h"
#include "IO/Modbus/modbus.h"
#include <QPixmap>

#define RESOLUTION_X		480
#define RESOLUTION_Y		272

class MainWindow : public QFrame
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = NULL);

signals:
    void sendkey(quint8);

public slots:
    void system_reboot();
    void setCloseTime(int m);
    void resetTimerFromKey();
    void showTime();

    void printSc(); //截屏


protected:
    void keyPressEvent(QKeyEvent *event);

private:
    KeyDetect *keydetect;
    MainMenu *mainmenu;
    FifoData *fifodata;
    Modbus *modbus;

    G_PARA *g_data;
    QTimer *rebootTimer;    //系统关机时间

    QTimer *showTimer;

    bool rebootFlag;

//    QPixmap *fullScreenPixmap;
};

#endif // MAINWINDOW_H
