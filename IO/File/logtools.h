#ifndef LOGTOOLS_H
#define LOGTOOLS_H

#include "IO/Data/data.h"
#include <QDateTime>
#include <QObject>
#include <QVector>
#include <qwt_point_3d.h>

#define MAX_LOG_NUM         5000    //保存日志最大条目数
#define LOG_TIME_INTERVAL   10      //记录时间间隔（10s）
#define SAVE_TIME_INTERVAL   60      //记录时间间隔（60s）

struct LOG_DATA {
    QString datetime;       //时间
    double val;             //测量值
    int pulse;              //2秒内脉冲数
    double degree;          //严重度
    uint is_current;        //是否是前台数据，1-前台，0-后台
    QString result;         //智能判断的结果，中文，如：噪声，局放
};

class LogTools : public QObject
{
    Q_OBJECT
public:
    explicit LogTools(MODE mode = TEV1, QObject *parent = 0);

    void read_normal_log(QString path, QVector<LOG_DATA> &log_data);        //读入历史数据，保存在链表中,适用于生成报告时读取数据
    void save_log();            //保存所有日志

public slots:
    void dealLog(double val, int pulse, double degree, int is_current = 0, QString result = "NOISE");     //处理日志文件（管理，保存）
    void dealRPRDLog(QVector<QwtPoint3D> points);
    void change_current_asset_dir();

private:


    MODE mode;                  //日志文件模式（由文件名辨识）
    QVector<LOG_DATA> data_normal;     //保存的链表
    QVector<LOG_DATA> data_asset;     //资产数据
    int log_timer;

    QString path_normal;               //文件路径
    QString path_asset;         //资产文件路径


    void write_normal_log(QString path, QVector<LOG_DATA> log_data);
    void save_PRPD_zdit(QVector<QwtPoint3D> points);                  //自定义PRPD标准
    void save_PRPD_State_Grid(QVector<QwtPoint3D> points);            //国网公司PRPD标准
};

#endif // LOGTOOLS_H








