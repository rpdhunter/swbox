#ifndef LOGTOOLS_H
#define LOGTOOLS_H

#include "IO/Data/data.h"
#include <QDateTime>
#include <QObject>
#include <QVector>
#include <qwt_point_3d.h>

#define MAX_LOG_NUM         10000    //保存日志最大条目数
#define LOG_TIME_INTERVAL   10      //记录时间间隔（10s）

class LogTools : public QObject
{
    Q_OBJECT
public:
    explicit LogTools(MODE mode, QObject *parent = 0);

public slots:
    void dealLog(double val, int pulse, double degree);     //处理日志文件（管理，保存）
    void dealRPRDLog(QVector<QwtPoint3D> points);

private:
    struct LOG_DATA {
        QString datetime;     //时间
        double val;             //测量值
        int pulse;              //2秒内脉冲数
        double degree;          //严重度
    };

    MODE mode;                  //日志文件模式（由文件名辨识）
    QVector<LOG_DATA> data;     //保存的链表
    int log_timer;

    void logFileInit();
    QString getFileName();
    void readFile();
    void writeFile();
};

#endif // LOGTOOLS_H
