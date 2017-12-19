#include "logtools.h"
#include <QFile>
#include <QDir>
#include <QtDebug>


LogTools::LogTools(MODE mode, QObject *parent) : QObject(parent)
{
    this->mode = mode;
    logFileInit();
    log_timer = 0;
}

void LogTools::dealLog(double val, int pulse, double degree)
{
    log_timer ++;   //计数器累加

    if(log_timer % LOG_TIME_INTERVAL == 0){
        //录入当前节点
        LOG_DATA d;
        d.datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz");
        d.val = val;
        d.pulse = pulse;
        d.degree = degree;
        data.append(d);

        //判断条目数
        while (data.length() > MAX_LOG_NUM) {
            data.removeFirst();
        }

        //保存文件
        writeFile();

        log_timer = 0;      //计数器清零
    }

}

void LogTools::dealRPRDLog(QVector<QwtPoint3D> points)
{
    qDebug()<<"PRPD files saved!";
    QFile file;
    bool flag;

    file.setFileName(PRPDLOG_DIR"/" + getFileName() + "_PRPD_" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz") + ".log");
    flag = file.open(QIODevice::WriteOnly);
    if(flag){
        QTextStream out(&file);

        foreach (QwtPoint3D p, points) {
            out << p.x() << "\t"
                << p.y() << "\t"
                << p.z() << "\n";
        }

        qDebug()<<"PRPD file saved! num = "<< points.length();

        file.close();
    }
}

void LogTools::logFileInit()
{
    //创建文件夹
    QDir dir;

    if(!dir.exists(DATA_DIR"/") ){
        dir.mkdir(DATA_DIR"/");
    }

    if(!dir.exists(DATALOG_DIR)){
        dir.mkdir(DATALOG_DIR);
    }

    if(!dir.exists(PRPDLOG_DIR)){
        dir.mkdir(PRPDLOG_DIR);
    }

    //读取文件
    readFile();
}

QString LogTools::getFileName()
{
    QString pre;

    switch (mode) {
    case TEV1:
        pre = "TEV1";
        break;
    case TEV2:
        pre = "TEV2";
        break;
    case AA_Ultrasonic:
        pre = "AA_Ultrasonic";
        break;
    case HFCT1:
        pre = "HFCT1";
        break;
    default:
        break;
    }
    return pre;
}

//读入历史数据，保存在链表中
void LogTools::readFile()
{
    QFile file;
    bool flag;

    file.setFileName(DATALOG_DIR"/" + getFileName() + "_NORMAL" + ".log");
    flag = file.open(QIODevice::ReadOnly);
    if(flag){
        QTextStream in(&file);

        LOG_DATA d;
        while (!in.atEnd()) {       //这种读法，需要测试
            in >> d.datetime >> d.val >> d.pulse >> d.degree;
            data.append(d);
        }
        if(!data.isEmpty())
            data.removeLast();  //移除最后一行

        file.close();
    }
}

void LogTools::writeFile()
{
    QFile file;
    bool flag;

    file.setFileName(DATALOG_DIR"/" + getFileName() + "_NORMAL" + ".log");
    flag = file.open(QIODevice::WriteOnly);
    if(flag){
        QTextStream out(&file);

        foreach (LOG_DATA d, data) {
            out << d.datetime << "\t"
                << d.val << "\t"
                << d.pulse << "\t"
                << d.degree << "\n";
        }

//        qDebug()<<"file saved! num = "<< data.length();

        file.close();
    }
}








