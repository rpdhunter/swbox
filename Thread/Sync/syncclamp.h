#ifndef SYNCCLAMP_H
#define SYNCCLAMP_H

#include <QThread>
#include <QVector>
#include <QTimer>
#include "IO/Other/CPU/cpustatus.h"
#include <sys/time.h>    // for gettimeofday()

class SyncClamp : public QThread
{
    Q_OBJECT
public:
    explicit SyncClamp(QObject *parent = nullptr);

    struct SYNC_DATA{
        struct timeval t;
        float vcc;
    };

signals:
    void update_syncBar(bool);  //参数为false代表同步未成功,true为同步成功
    void send_sync(qint64, qint64);     //发送过零点的秒和微秒(linux timeval格式)
    void update_statusBar(QString);

public slots:
    void write_to_rdb();
    void change_thread_status();

protected:
    void run();

private:
    QTimer *timer;
    CPUStatus *cpu_status;
    QVector<SYNC_DATA> sync_data;       //同步数据信号
    QVector<timeval> zero_times;         //保存之前一系列过零点

    void get_current_sync_data();
};

#endif // SYNCCLAMP_H
