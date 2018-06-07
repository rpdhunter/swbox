#include "logtools.h"
#include <QFile>
#include <QDir>
#include <QtDebug>
#include "IO/SqlCfg/sqlcfg.h"
#include "Gui/Common/common.h"


LogTools::LogTools(MODE mode, QObject *parent) : QObject(parent)
{
    this->mode = mode;

    log_timer = 0;
    path_normal = DIR_DATALOG"/" + Common::MODE_toString(mode) + "_NORMAL" + ".log";
    path_asset = DIR_ASSET_NORMAL"/" + Common::MODE_toString(mode) + "_NORMAL" + ".log";

    //读取文件,资产文件不需要在初始化时读取
    read_normal_log(path_normal,data_normal);
}

//完成2个功能：
//1.保存一般测量数据至公共空间
//2.保存一般测量数据至资产空间，资产空间的数据是公共空间数据的子集
void LogTools::dealLog(double val, int pulse, double degree, int is_current)
{
    log_timer ++;   //计数器累加

    if(log_timer % LOG_TIME_INTERVAL == 0 || qAbs((val-data_normal.last().val)/data_normal.last().val) > 0.2){
        //录入当前节点
        LOG_DATA d;
        d.datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz");
        d.val = val;
        d.pulse = pulse;
        d.degree = degree;
        d.is_current = is_current;
        data_normal.append(d);
        data_asset.append(d);

        //判断条目数
        while (data_normal.length() > MAX_LOG_NUM) {
            data_normal.removeFirst();
        }
        while (data_asset.length() > MAX_LOG_NUM) {
            data_asset.removeFirst();
        }

        //保存文件
        write_normal_log(path_normal,data_normal);
        write_normal_log(path_asset,data_asset);
    }
    if(log_timer == LOG_TIME_INTERVAL){
        log_timer = 0;      //计数器清零
    }
}

//完成PRPD文件的保存
//一般测量保存至标准目录
//资产测量保存至资产目录（以硬链接的形式）
void LogTools::dealRPRDLog(QVector<QwtPoint3D> points)
{
    if(points.isEmpty()){
        return;
    }
    QFile file;
    bool flag;

    QString file_name = QString(Common::MODE_toString(mode) + "_PRPD_" + QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss-zzz") + ".log");
    file.setFileName(DIR_PRPDLOG"/" + file_name);

    QString str_src = file.fileName();
    flag = file.open(QIODevice::WriteOnly);
    if(flag){
        QTextStream out(&file);

        foreach (QwtPoint3D p, points) {
            out << p.x() << "\t"
                << p.y() << "\t"
                << p.z() << "\n";
        }

        qDebug()<<"PRPD file saved! \tnum="<< points.length() << "\tmode = "<<Common::MODE_toString(mode);

        file.close();

        Common::create_hard_link(str_src, file_name);           //建立硬连接

    }
}

void LogTools::change_current_asset_dir()
{
    path_asset = QString::fromLocal8Bit(sqlcfg->get_para()->current_dir) + "/" + Common::MODE_toString(mode) + "_NORMAL" + ".log";
//    printf("current dir:%s\n", path_asset.toLocal8Bit().data());
    read_normal_log(path_normal,data_normal);
}

//读入历史数据，保存在链表中
void LogTools::read_normal_log(QString path, QVector<LOG_DATA> &log_data)
{
    QFile file;
    file.setFileName(path);
    bool flag = file.open(QIODevice::ReadOnly);

    log_data.clear();
    if(flag){
        QTextStream in(&file);
        LOG_DATA d;
        for (int i = 0; i < MAX_LOG_NUM; ++i) {
            if (!in.atEnd()) {
                in >> d.datetime >> d.val >> d.pulse >> d.degree >> d.is_current;
                log_data.append(d);
            }
            else{
                break;
            }
        }

        if(!log_data.isEmpty())
            log_data.removeLast();  //移除最后一行

        file.close();
    }
    else{
        qDebug()<<"read file failed!";

    }
}

void LogTools::write_normal_log(QString path, QVector<LOG_DATA> log_data)
{
    QFile file;
    file.setFileName(path);
    bool flag = file.open(QIODevice::WriteOnly);
//    if(mode == TEV1)
//        printf("writeFile:%s\n", path.toLocal8Bit().data());

    if(flag){
        QTextStream out(&file);

        foreach (LOG_DATA d, log_data) {
            out << d.datetime << "\t"
                << d.val << "\t"
                << d.pulse << "\t"
                << d.degree << "\t"
                << d.is_current << "\n";
        }

        file.close();
    }
    else{
        qDebug()<<"log file saved failed! ";
    }
}








