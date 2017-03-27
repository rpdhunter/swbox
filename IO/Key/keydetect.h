#ifndef KEYDETECT_H
#define KEYDETECT_H

#include "key.h"
#include <QThread>
#include <QString>
#include <QDebug>
#include <QKeyEvent>
#include <QTimer>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct KEY_SYS {
    int exportfd;
    int valuefd[KEY_NUM];
    int directionfd[KEY_NUM];
} KEY_SYS;


//键盘监测线程
//与外部接口为键盘按键信号
class KeyDetect : public QThread
{
    Q_OBJECT
public:
    explicit KeyDetect(QObject *parent = NULL);

protected:
    void run(void);

    quint8 key_value, key_send;

signals:
    void sendkey(quint8);

public slots:

private:

    int cnt;

    char *base_addr;

    void set_dir_pin(int pin, int dir);
    void get_bank(int pin, int *bank);
    void set_output_enable(int pin, int enable);
    bool gpio_read_pin(int pin);
};

#endif // KEYDETECT_H
